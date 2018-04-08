
#include "AgPointCloudProject.h"
#include "../AgSceneDatabase.h"
#include "AgPointCloudLoadThread.h"

#include <algorithm>

#include <common/RCMemoryHelper.h>
#include <utility/RCMemory.h>

using namespace ambergris;
using namespace ambergris::RealityComputing::Utility;
using namespace ambergris::RealityComputing::Utility::Threading;
using namespace ambergris::RealityComputing::Common;

namespace {

	struct ScanStats
	{
		ScanStats() : voxelCount(0), pointCount(0) {}

		size_t voxelCount;
		size_t pointCount;

		size_t pointsPerVoxel() const { return pointCount / voxelCount; }
	};

	class VisibleNodesLODSorterFunctor
	{
	public:
		VisibleNodesLODSorterFunctor(const std::vector< ScanStats >& scanPointCounts)
			: mScanPointCounts(scanPointCounts)
		{
		}

		bool operator() (ScanContainerID* p1, const ScanContainerID* p2) const
		{
			// Prefer scans that had a higher point count per voxel
			const size_t scanCount1 = mScanPointCounts[p1->m_scanId].pointsPerVoxel();
			const size_t scanCount2 = mScanPointCounts[p2->m_scanId].pointsPerVoxel();
			if (scanCount1 > scanCount2)
				return true;
			else if (scanCount1 < scanCount2)
				return false;

			// If there is a tie, choose scans deterministically.
			if (p1->m_scanId < p2->m_scanId)
				return true;
			else if (p1->m_scanId > p2->m_scanId)
				return false;

			//// Choose based on LOD
			//if( p1->m_LODs[0].m_LOD > p2->m_LODs[0].m_LOD )
			//    return true;     
			//else if( p1->m_LODs[0].m_LOD < p2->m_LODs[0].m_LOD )
			//    return false;

			// Choose based on container ID
			if (p1->m_containerId < p2->m_containerId)
				return true;
			else if (p1->m_containerId > p2->m_containerId)
				return false;

			return false;
		}

	private:
		const std::vector< ScanStats >& mScanPointCounts;
	};
}

WorkerThread::WorkerThread()
{
	mEndThread = true;
	mRunning = false;
	mpThread = new tbb::tbb_thread();
}

WorkerThread::~WorkerThread()
{
	Stop();
	DeletePtr(mpThread);
}

void WorkerThread::Join()
{
	if (mpThread && mpThread->joinable())
		mpThread->join();
}

bool WorkerThread::IsRunning() const
{
	return  mRunning;
}

bool WorkerThread::Start()
{
	// Let the thread run.
	mEndThread = false;

	// Already running - return false.
	if (mRunning)
		return false;

	tbb::tbb_thread worker(Run, this);
	if (!worker.joinable())
		return false;

	*mpThread = std::move(worker);

	return true;
}

void WorkerThread::Run(WorkerThread * worker)
{
	worker->mRunning = true;
	worker->doTask();
	worker->mRunning = false;
}

void WorkerThread::Stop()
{
	mEndThread = true;
	Join();
}

AgPointCloudRequests::AgPointCloudRequests()
{
	mInterruptLoading = false;
	mSortNodes = true;
	mStatus = Status::Inactive;
	mRequestSetMutex = rc_make_unique<RCMutex>();
}

AgPointCloudRequests::~AgPointCloudRequests()
{
}

bool AgPointCloudRequests::Start()
{
	mInterruptLoading = false;
	return WorkerThread::Start();
}

AgPointCloudRequests::Status AgPointCloudRequests::GetStatus() const
{
	return mStatus;
}

void AgPointCloudRequests::Clear()
{
	mInterruptLoading = true;
	Stop();
	RCScopedLock<RCMutex> lock(*mRequestSetMutex);
	mRequestSet.clear();
	mStatus = Status::Inactive;
}

void AgPointCloudRequests::doTask()
{
	RCScopedLock<RCMutex> lock(*mRequestSetMutex);

	//free cache once
	AgPointCloudProject& pointCloud = Singleton<AgSceneDatabase>::instance().getPointCloudProject();
	pointCloud.freeLRUCache();
	if (mSortNodes) //sort nodes based on their LOD?
	{
		//it is faster to sort with pointers to struct then using them sefl
		//TODO is there an alternative???
		// Also, count the number of points and voxels per scan
		std::vector< ScanStats > scanPointCounts(pointCloud.getSize());
		std::vector<ScanContainerID*> resultList(mRequestSet.size());
		for (size_t i = 0; i < mRequestSet.size(); ++i)
		{
			auto& cont = mRequestSet[i];
			resultList[i] = &cont;
			ScanStats& stats = scanPointCounts[cont.m_scanId];
			for (auto& lod : cont.m_LODs)
			{
				stats.pointCount += lod.m_desiredPointCount;
				stats.voxelCount++;
			}
		}

		//sort
		std::sort(resultList.begin(), resultList.end(), VisibleNodesLODSorterFunctor(scanPointCounts));
		//copy back
		std::vector<ScanContainerID> copyList(mRequestSet.size());
		for (size_t i = 0; i < resultList.size(); ++i)
		{
			//std::stringstream ss;
			//ss << "s" << resultList[i]->m_scanId << ", c" << resultList[i]->m_containerId << ", l" << (int)(resultList[i]->m_LODs[0].m_LOD) << ", p" << resultList[i]->m_LODs[0].m_pointCount << std::endl;
			//OutputDebugStringA( ss.str().c_str() );
			copyList[i] = *resultList[i];
		}

		const auto complete = (pointCloud.refineVoxels(copyList, -1, mInterruptLoading, NULL) != 0.0f);
		if (complete)
			mStatus = Status::Complete;
	}
	else
	{
		const auto complete = (pointCloud.refineVoxels(mRequestSet, -1, mInterruptLoading, NULL) != 0.0f);
		if (complete)
			mStatus = Status::Complete;
	}
}

void AgPointCloudRequests::SetWorkingSet(const std::vector<ScanContainerID>& newRequest)
{
	RCScopedLock<RCMutex> lock(*mRequestSetMutex);
	mRequestSet = newRequest;
	mStatus = Status::Active;
}

void AgPointCloudRequests::SetSortNodes(bool val)
{
	mSortNodes = val;
}