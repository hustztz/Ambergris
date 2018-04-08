#include "AgVoxelContainer.h"
#include "AgVoxelTreeRunTime.h"
#include "AgVoxelTreeRawPointConverter.h"
#include "../AgSceneDatabase.h"

#include <common/RCMemoryMapFile.h>
#include <utility/RCMemory.h>
#include <utility/RCAssert.h>
#include <utility/RCSpatialReference.h>

using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility;
using namespace ambergris::RealityComputing::Utility::Threading;

namespace ambergris {

	AgVoxelContainer::AgVoxelContainer(AgVoxelTreeRunTime* parentTreePtr) :
		m_hasOccluder(false),
		m_parentTreeHandle(AgVoxelTreeRunTime::kInvalidHandle),
		m_currentLODLoaded(-1),
		m_currentDrawLOD(0),
		m_maximumLOD(0),
		m_nodeOffsetStart(0),
		m_pointDataOffsetStart(0),
		m_segmentOffsetStarts(0),
		m_timeStampOffsetStarts(0),
		m_viewId(0),
		m_visibleListId(-1),
		m_lastTimeModified(0),
		m_nodeRadius(0),
		m_numOldPoints(0),
		m_loadMutexPtr(new RCMutex),
		m_diskMutexPtr(new RCMutex),
		m_clipFlag(NON_CLIPPED),
		mExternalClipFlag(NON_CLIPPED),
		mInternalClipFlag(NON_CLIPPED),
		m_nClipIndex(0),
		m_regionFlag(0),
		mHasPersistentDeletedPts(false)
	{
		//setRegionIndex(alPointSelect::UNASSIGNED);
		setIsInTempSelection(false);

		memset(&m_amountOfLODPoints[0], 0, sizeof(int) * 32);
		memset(&m_amountOfOctreeNodes[0], 0, sizeof(int) * 32);
	}

	void AgVoxelContainer::loadTerrestialLODInternal(int newLOD, RCMemoryMapFile* memMapFile, bool lock)
	{
		int amountOfPoints = m_amountOfLODPoints[newLOD];
		if (amountOfPoints <= 0)
			return;

		//only stream in additional points
		int oldPoints = 0;
		int startLod = 0;

		/*alViewport* viewportPtr = m_parentTreePtr->getWorld()->getActiveViewport();
		if (viewportPtr && (viewportPtr->getPointCloudDisplayType() == POINTCLOUD_DISPLAY_SEGMENTS || viewportPtr->testGetShowSegment()))
		{
			oldPoints = 0;
			startLod = 0;
		}
		else*/
		{
			if (m_currentLODLoaded > 0) //only load in additional data, not starting from beginning!
			{
				oldPoints = m_amountOfLODPoints[m_currentLODLoaded];
				startLod = m_currentLODLoaded;
			}
		}

		m_numOldPoints = oldPoints;

		//new points to be streamed in
		int pointsToBeStreamedIn = amountOfPoints - oldPoints;
		if (pointsToBeStreamedIn <= 0)
			return;

		std::vector<AgVoxelLeafNode> rawPoints(pointsToBeStreamedIn);
		std::vector<std::uint16_t> segIdList(pointsToBeStreamedIn);

		{
			std::unique_ptr<RCScopedLock<RCMutex> > diskLock = lock ? rc_make_unique<RCScopedLock<RCMutex> >(*m_diskMutexPtr) : nullptr;

			if (!memMapFile)
			{
				RCMemoryMapFile memmap(m_parentTreePtr->getFileName().c_str());

				if (memmap.createFileHandleOnlyRead())
				{
					memmap.setFilePointer(m_pointDataOffsetStart);
					memmap.readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * pointsToBeStreamedIn);

					if (m_segmentOffsetStarts) {
						memmap.setFilePointer(m_segmentOffsetStarts);
						memmap.readFile(&segIdList[0], sizeof(std::uint16_t) * pointsToBeStreamedIn);
					}
				}
			}
			else
			{
				memMapFile->setFilePointer(m_pointDataOffsetStart + (sizeof(AgVoxelLeafNode) * oldPoints));
				bool good = memMapFile->readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * pointsToBeStreamedIn);
				if (!good)
				{
					RCASSERTCORE(false, "Could not read voxel data");
					return;
				}

				if (m_segmentOffsetStarts) {
					memMapFile->setFilePointer(m_segmentOffsetStarts + (sizeof(std::uint16_t) * oldPoints));
					good = memMapFile->readFile(&segIdList[0], sizeof(std::uint16_t) * pointsToBeStreamedIn);
					if (!good)
					{
						RCASSERTCORE(false, "Could not read segment data");
						return;
					}
				}

				/*if (viewportPtr && (viewportPtr->getPointCloudDisplayType() == POINTCLOUD_DISPLAY_SEGMENTS || viewportPtr->testGetShowSegment()))
				{
					if (m_segmentOffsetStarts)
					{
						for (int i = 0; i < pointsToBeStreamedIn; i++)
						{
							auto& rawPoint = rawPoints[i];
							RCVector3f color = PointRegionManager::getDefaultRegionColorForIndex((3 * (segIdList[i] + 1)) % 255);
							rawPoint.setRGBA(RCVector4ub((std::uint8_t)color.x, (std::uint8_t)color.y, (std::uint8_t) color.z, 255));
						}
					}
				}*/
			}
		}

		{

			std::unique_ptr<RCScopedLock<RCMutex> > loadLock = lock ? rc_make_unique<RCScopedLock<RCMutex> >(*m_loadMutexPtr) : nullptr;

			m_terrestialPointList.resize(amountOfPoints);
			m_terrestialSegIdList.resize(amountOfPoints);

			//convert to internal format, starting from previous amount of points   
			bool hasDeletedPts = false;
			for (int i = 0; i < pointsToBeStreamedIn; i++)
			{
				auto& point = m_terrestialPointList[i + oldPoints];
				auto& rawPoint = rawPoints[i];
				point.setRawCoord(rawPoint.getRawOffsetFromBoundingBox());

				if (m_parentTreePtr->hasIntensity())
					point.setRGBA(rawPoint.getRGBA());
				else
				{
					RCVector4ub color = rawPoint.getRGBA();
					point.setRGBA(RCVector4ub((std::uint8_t)color.x, (std::uint8_t)color.y, (std::uint8_t) color.z, (std::uint8_t) 1));
				}

				point.setNormalIndex(rawPoint.getNormal());

				if (rawPoint.getDeleteFlag()) {
					point.setFiltered(true);
					hasDeletedPts = true;
				}

				if (m_segmentOffsetStarts)
					m_terrestialSegIdList[i + oldPoints] = segIdList[i];
				else
					m_terrestialSegIdList[i + oldPoints] = 65535;
			}

			if (hasDeletedPts) {
				//set flags and for both the voxel and the parent scan
				setHasPeristentDeletedPts(true);
				//m_parentTreePtr->setHasPeristentDeletedPts(true);
			}

			//applyCrop();
			//applySelections(oldPoints);   

			m_currentLODLoaded = newLOD;
			m_currentDrawLOD = newLOD;

			if (!Singleton<AgSceneDatabase>::instance().m_pointCloudManager.getLightWeight())
			{

				//RCTimer regionTimer;
				//if (RCRegionConfig::isTimeLogEnabled())
				//    regionTimer.start();

				updateLayerEffectWhenRefinePoints(oldPoints);

				if (!Singleton<AgSceneDatabase>::instance().m_pointCloudManager.getIgnoreClip())
					updateClipEffectWhenRefinePoints(oldPoints);

				//if (RCRegionConfig::isTimeLogEnabled())
				//{
				//    regionTimer.stop();
				//    RCLog::getLogFile()->addMessage(regionTimer.printElapsedTime(L" voxel apply region time cost:"));
				//}
			}
		}
	}

	/*void AgVoxelContainer::clearPersistentDeleteFlags(RCMemoryMapFile* memMapFile)
	{
		if (!getHasPeristentDeletedPts())
			return;
		//recover LOD info
		if (m_maximumLOD == 1)
		{
			//modify the max LOD info in this voxel
			OctreeFileChunk umbrellaChunk;
			if (!m_parentTreePtr->getFileChunk(ENUM_FILE_CHUNK_ID_UMBRELLA_OCTREE_DATA, umbrellaChunk))
				return;
			memMapFile->setFilePointer(umbrellaChunk.m_fileOffsetStart + sizeof(std::uint64_t) + m_originalIndex * sizeof(OctreeLeafSaveData));
			OctreeLeafSaveData leafSaveData;
			memMapFile->readFile(&leafSaveData, sizeof(OctreeLeafSaveData));

			int cumLeafPoints = 0;
			int cumNodePoints = 0;

			//clear the data inside this voxel
			for (int jLevel = 0; jLevel < 32; jLevel++)
			{
				//cumulative stuff
				cumLeafPoints += leafSaveData.m_numLeafNodes[jLevel];
				cumNodePoints += leafSaveData.m_numLeafNodes[jLevel] + leafSaveData.m_numNonLeafNodes[jLevel];
				m_amountOfOctreeNodes[jLevel] = leafSaveData.m_numLeafNodes[jLevel] + leafSaveData.m_numNonLeafNodes[jLevel];
				m_amountOfLODPoints[jLevel] = leafSaveData.m_numNonLeafNodes[jLevel] + cumLeafPoints;
			}

			int maxDepth = 0;
			for (int jLevel = 0; jLevel < 32; ++jLevel)
			{
				if (maxDepth == 0 && m_amountOfOctreeNodes[jLevel] == 0)
				{
					maxDepth = jLevel;
					break;
				}
			}

			leafSaveData.m_maxDepth = maxDepth;
			memMapFile->setFilePointer(umbrellaChunk.m_fileOffsetStart + sizeof(std::uint64_t) + m_originalIndex * sizeof(OctreeLeafSaveData));
			memMapFile->writeFile(&leafSaveData, sizeof(OctreeLeafSaveData));

			m_currentLODLoaded = -1;
			m_currentDrawLOD = -1;
			m_terrestialPointList.clear();
			m_lidarPointList.clear();
		}

		int LODlevel = 31; //all levels
		bool updateRequired = false;
		std::vector<AgVoxelLeafNode> leafNodes; leafNodes.reserve(m_amountOfLODPoints[LODlevel]);

		if (!m_parentTreePtr->isLidarData())
		{
			loadTerrestialLODInternal(LODlevel, memMapFile, true);

			//set the delete flag on all points that are in the deleted region or on every point if the whole voxel or scan is deleted
			for (auto& pt : m_terrestialPointList) {
				AgVoxelLeafNode node;
				populateNode(node, pt);
				if (pt.isFiltered()) {
					pt.setFiltered(false);
					node.setDeleteFlag(false);
					updateRequired = true;
				}
				leafNodes.push_back(node);
			}
		}
		else
		{
			loadLidarLODInternal(LODlevel, memMapFile, true);
			for (auto& pt : m_lidarPointList) {
				AgVoxelLeafNode node;
				populateNode(node, pt);
				if (pt.isFiltered()) {
					pt.setFiltered(false);
					node.setDeleteFlag(false);
					updateRequired = true;
				}
				leafNodes.push_back(node);
			}
		}

		//if any points in the voxel needed a flag set, then write the whole voxel
		if (updateRequired) {
			//write the leaf nodes
			memMapFile->setFilePointer(m_pointDataOffsetStart);
			if (!leafNodes.empty()) {
				memMapFile->writeFile(&leafNodes[0], sizeof(AgVoxelLeafNode) * static_cast<int>(leafNodes.size()));
			}
			setHasPeristentDeletedPts(false);
			m_parentTreePtr->setHasPeristentDeletedPts(false);
		}
	}

	void AgVoxelContainer::updatePersistentDeleteFlags(RCMemoryMapFile* memMapFile) {
		//if there is no point deleted in this voxel, skip this voxel
		if (isRegionFlagValid() && getInheritRegionIndex() != alPointSelect::SELECTION_AND_DELETE_POINTS)
			return;

		int LODlevel = 31; //all levels
		bool updateRequired = false;
		std::vector<AgVoxelLeafNode> leafNodes; leafNodes.reserve(m_amountOfLODPoints[LODlevel]);
		bool wholeVoxelDeleted = getRegionIndex() == alPointSelect::SELECTION_AND_DELETE_POINTS ||
			m_parentTreePtr->getRegionIndex() == alPointSelect::SELECTION_AND_DELETE_POINTS;

		if (!m_parentTreePtr->isLidarData())
		{
			loadTerrestialLODInternal(LODlevel, memMapFile, true);

			//set the delete flag on all points that are in the deleted region or on every point if the whole voxel or scan is deleted
			for (auto& pt : m_terrestialPointList) {
				AgVoxelLeafNode node;
				populateNode(node, pt);
				if (wholeVoxelDeleted || pt.getLayerInfo() == alPointSelect::SELECTION_AND_DELETE_POINTS) {
					pt.setFiltered(true);
					node.setDeleteFlag(true);
					updateRequired = true;
				}
				leafNodes.push_back(node);
			}
		}
		else
		{
			loadLidarLODInternal(LODlevel, memMapFile, true);
			for (auto& pt : m_lidarPointList) {
				AgVoxelLeafNode node;
				populateNode(node, pt);
				if (wholeVoxelDeleted || pt.getLayerInfo() == alPointSelect::SELECTION_AND_DELETE_POINTS) {
					pt.setFiltered(true);
					node.setDeleteFlag(true);
					updateRequired = true;
				}
				leafNodes.push_back(node);
			}
		}

		//if any points in the voxel needed a flag set, then write the whole voxel
		if (updateRequired) {

			if (wholeVoxelDeleted)
			{
				//modify the max LOD info in this voxel
				OctreeFileChunk umbrellaChunk;
				if (!m_parentTreePtr->getFileChunk(ENUM_FILE_CHUNK_ID_UMBRELLA_OCTREE_DATA, umbrellaChunk))
					return;
				memMapFile->setFilePointer(umbrellaChunk.m_fileOffsetStart + sizeof(std::uint64_t) + m_originalIndex * sizeof(OctreeLeafSaveData));
				OctreeLeafSaveData leafSaveData;
				memMapFile->readFile(&leafSaveData, sizeof(OctreeLeafSaveData));
				leafSaveData.m_maxDepth = 1;
				memMapFile->setFilePointer(umbrellaChunk.m_fileOffsetStart + sizeof(std::uint64_t) + m_originalIndex * sizeof(OctreeLeafSaveData));
				memMapFile->writeFile(&leafSaveData, sizeof(OctreeLeafSaveData));

				//clear the data inside this voxel
				m_terrestialPointList.clear();
				m_lidarPointList.clear();
				m_maximumLOD = 1;
				for (int iLevel = m_maximumLOD; iLevel < 32; ++iLevel)
				{
					m_amountOfOctreeNodes[iLevel] = 0;
					m_amountOfLODPoints[iLevel] = m_amountOfLODPoints[m_maximumLOD - 1];
				}
			}
			memMapFile->setFilePointer(m_pointDataOffsetStart);
			if (!leafNodes.empty()) {
				memMapFile->writeFile(&leafNodes[0], sizeof(AgVoxelLeafNode) * static_cast<int>(leafNodes.size()));
			}
			setHasPeristentDeletedPts(true);
			m_parentTreePtr->setHasPeristentDeletedPts(true);
		}
	}

	bool AgVoxelContainer::generateVoxelTempFiles(RCMemoryMapFile* memMapFile, const std::wstring& tempFile, const std::wstring& tempFolder, AgVoxelContainerStatus& voxelStatus,
		OctreeIntermediateNode<BasicOctreeImportPoint>& intermediateNode)
	{
		//if no points in the voxel container are deleted
		if (getHasPeristentDeletedPts() == false
			&& isRegionFlagValid() && getInheritRegionIndex() != alPointSelect::SELECTION_AND_DELETE_POINTS)
		{
			voxelStatus = AgVoxelContainerStatus::UNCHANGED;
			return true;
		}

		int LODlevel = 31; //all levels
		bool isTerrestrial = !m_parentTreePtr->isLidarData();

		size_t filteredPointSize = 0;
		size_t totalPointsInVoxel = 0;

		int oldPoints = m_amountOfLODPoints[m_currentLODLoaded];    //back up already loaded point index

		if (isTerrestrial)
		{
			loadTerrestialLODInternal(LODlevel, memMapFile, true);     //todo: just apply deletion filters to all the points in the voxel container, otherwise this will be slow when there are many regions
																	   //if the selection effect is  not updated during loading because canDrawWorld is false, force to apply selection effect here
			if (m_parentTreePtr->getWorld()->getCanDrawWorld() == false)
			{
				m_parentTreePtr->getWorld()->getPointSelectionManager() ->
					refineSelectionEffectOnAgVoxelContainer(this, oldPoints);
			}

			totalPointsInVoxel = m_terrestialPointList.size();
			for (auto& pnt : m_terrestialPointList)
			{
				if (pnt.isDeleted(this) || pnt.isFiltered())
				{
					++filteredPointSize;
				}
			}
		}
		else
		{
			loadLidarLODInternal(LODlevel, memMapFile, true);
			//if the selection effect is  not updated during loading because canDrawWorld is false, force to apply selection effect here
			if (m_parentTreePtr->getWorld()->getCanDrawWorld() == false)
			{
				m_parentTreePtr->getWorld()->getPointSelectionManager() ->
					refineSelectionEffectOnAgVoxelContainer(this, oldPoints);
			}

			totalPointsInVoxel = m_lidarPointList.size();
			for (auto& pnt : m_lidarPointList)
			{
				if (pnt.isDeleted(this) || pnt.isFiltered())
				{
					++filteredPointSize;
				}
			}
		}


		//todo: consider use morton code
		if (filteredPointSize == 0)
		{
			voxelStatus = VoxelContainerStatus::UNCHANGED;
		}
		else if (filteredPointSize == totalPointsInVoxel)
		{
			voxelStatus = VoxelContainerStatus::REMOVED;
		}
		else
		{
			voxelStatus = VoxelContainerStatus::REINDEXED;

			std::vector<BasicOctreeImportPoint> octreePts;
			octreePts.reserve(m_amountOfLODPoints[LODlevel]);

			if (isTerrestrial)
			{
				for (size_t idx = 0; idx < m_terrestialPointList.size(); idx++) {

					if (m_terrestialPointList[idx].isDeleted(this) || m_terrestialPointList[idx].isFiltered())
						continue;

					BasicOctreeImportPoint octreePt;
					populateBasicOctreePt(octreePt, m_terrestialPointList[idx], m_svoBounds.getMin(), m_terrestialSegIdList[idx]);
					octreePts.push_back(octreePt);

					intermediateNode.updatePointInformation(octreePt);
				}
			}
			else
			{
				for (size_t idx = 0; idx < m_lidarPointList.size(); idx++) {

					if (m_lidarPointList[idx].isDeleted(this) || m_lidarPointList[idx].isFiltered())
						continue;

					BasicOctreeImportPoint octreePt;
					populateBasicOctreePt(octreePt, m_lidarPointList[idx], m_svoBounds.getMin());
					octreePts.push_back(octreePt);

					intermediateNode.updatePointInformation(octreePt);
				}
			}

			intermediateNode.m_svoBounds = m_svoBounds;    //todo: need update or not? - Yan Fu
			intermediateNode.m_fileName = tempFile;

			//re-create octree for remaining points in this voxel container
			VoxelOctreeCreator<BasicOctreeImportPoint> creator(octreePts, tempFile, tempFolder, m_svoBounds);
			creator.setHasNormal(true);     //avoid re-computing normal information
			if (creator.createOctree())
			{
				creator.saveToDisk(&intermediateNode);
			}
			else
			{
				return false;
			}
		}

		return true;
	}

	bool AgVoxelContainer::writeToDisk()
	{
		RCScopedLock< RCMutex > loadLock(*m_loadMutexPtr);
		RCScopedLock< RCMutex > diskLock(*m_diskMutexPtr);

		std::vector<AgVoxelLeafNode> rawPoints;
		if (m_parentTreePtr->isLidarData())
		{
			rawPoints.resize(m_lidarPointList.size());
			for (size_t i = 0; i != rawPoints.size(); ++i)
			{
				const auto& ep = m_lidarPointList[i];
				AgVoxelLeafNode& rp = rawPoints[i];

				rp.setRawOffsetFromBoundingBox(ep.getRawCoord());

				rp.setRGBA(ep.getRGBA());
				rp.setNormal(static_cast<uint16_t>(ep.getNormalIndex()));
				rp.setLidarClassification(ep.getLidarClassification());
				rp.setDeleteFlag(ep.isDeleted(this) || ep.isFiltered());
			}
		}
		else
		{
			rawPoints.resize(m_terrestialPointList.size());
			for (size_t i = 0; i != rawPoints.size(); ++i)
			{
				const auto& ep = m_terrestialPointList[i];
				AgVoxelLeafNode& rp = rawPoints[i];

				rp.setRawOffsetFromBoundingBox(ep.getRawCoord());

				rp.setRGBA(ep.getRGBA());
				rp.setNormal(static_cast<uint16_t>(ep.getNormalIndex()));
				rp.setLidarClassification(ep.getLidarClassification());
				rp.setDeleteFlag(ep.isDeleted(this) || ep.isFiltered());
			}
		}
		if (rawPoints.empty())
		{
			RCASSERTCORE(false, "No points to write");
			return false;
		}

		// Tell the engine state to close it's file handles
		auto* pEngineState = m_parentTreePtr->getWorld();
		std::wstring fileId = m_parentTreePtr->getId();
		bool good = pEngineState->closeFileStream(fileId);
		if (!good)
		{
			RCASSERTCORE(false, "Couldn't close existing read-only file handle");
			return false;
		}

		RealityComputing::Common::RCMemoryMapFile memMapFile(m_parentTreePtr->getFileName().c_str());
		good = memMapFile.createFileHandle();
		if (good)
		{
			memMapFile.setFilePointer(m_pointDataOffsetStart);
			good = memMapFile.writeFile(&rawPoints[0], static_cast<int>(sizeof(AgVoxelLeafNode) * rawPoints.size()));
			memMapFile.closeFileHandle();
		}

		// re-open the file stream
		pEngineState->openFileStream(fileId, m_parentTreePtr->getFileName()); //reopen the read only file handle
		return good;
	}*/


	void AgVoxelContainer::loadLidarLODInternal(int lodLevel, RCMemoryMapFile* memMapFile, bool lock)
	{
		int amountOfPoints = m_amountOfLODPoints[lodLevel];
		if (amountOfPoints <= 0)
			return;
		//only stream in additional points
		int oldPoints = 0;
		int startLod = 0;
		if (m_currentLODLoaded > 0) //only load in additional data, not starting from beginning!
		{
			oldPoints = m_amountOfLODPoints[m_currentLODLoaded];
			startLod = m_currentLODLoaded;
		}
		m_numOldPoints = oldPoints;

		//new points to be streamed in
		int pointsToBeStreamedIn = amountOfPoints - oldPoints;
		if (pointsToBeStreamedIn <= 0)
			return;


		std::vector<AgVoxelLeafNode> rawPoints(pointsToBeStreamedIn);

		std::vector<double> timeStampData;
		if (m_timeStampOffsetStarts)
			timeStampData.resize(pointsToBeStreamedIn);

		{
			std::unique_ptr<RCScopedLock<RCMutex> > diskLock = lock ? rc_make_unique<RCScopedLock<RCMutex> >(*m_diskMutexPtr) : nullptr;

			if (!memMapFile)
			{
				RCMemoryMapFile memmap(m_parentTreePtr->getFileName().c_str());

				if (memmap.createFileHandleOnlyRead())
				{
					memmap.setFilePointer(m_pointDataOffsetStart);
					memmap.readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * amountOfPoints);
				}

				if (m_timeStampOffsetStarts) {
					memmap.setFilePointer(m_timeStampOffsetStarts + (sizeof(double) * oldPoints));
					memmap.readFile(&timeStampData[0], sizeof(double) * pointsToBeStreamedIn);
				}

			}
			else
			{
				memMapFile->setFilePointer(m_pointDataOffsetStart + (sizeof(AgVoxelLeafNode) * oldPoints));
				memMapFile->readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * pointsToBeStreamedIn);

				if (m_timeStampOffsetStarts) {
					memMapFile->setFilePointer(m_timeStampOffsetStarts + (sizeof(double) * oldPoints));
					memMapFile->readFile(&timeStampData[0], sizeof(double) * pointsToBeStreamedIn);
				}
			}
		}

		{
			std::unique_ptr<RCScopedLock<RCMutex> > loadLock = lock ? rc_make_unique<RCScopedLock<RCMutex> >(*m_loadMutexPtr) : nullptr;

			//resize
			m_lidarPointList.resize(amountOfPoints);

			if (m_timeStampOffsetStarts)
				m_timeStampList.resize(amountOfPoints);

			//convert to internal format
			bool hasDeletedPts = false;

			RCSpatialReference* pMentorTransformation = m_parentTreePtr->getMentorTransformation();
			for (int i = 0; i < pointsToBeStreamedIn; i++)
			{
				auto& point = m_lidarPointList[i + oldPoints];
				auto& rawPoint = rawPoints[i];
				if (pMentorTransformation != NULL)
				{
					auto coord = AgVoxelTreeRawPointConverter::rawPointToWorldCoord(this, rawPoint);

					coord = Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.globalToWorld(coord);
					pMentorTransformation->transform(coord);
					coord = Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.worldToGlobal(coord);

					point.setRawCoord(AgVoxelTreeRawPointConverter::worldCoordToRawPoint(this, coord).convertTo<float>());
				}
				else
				{
					point.setRawCoord(rawPoint.getRawOffsetFromBoundingBox());
				}

				if (m_parentTreePtr->hasIntensity())
					point.setRGBA(rawPoint.getRGBA());
				else
				{
					RCVector4ub color = rawPoint.getRGBA();
					point.setRGBA(RCVector4ub((std::uint8_t)color.x, (std::uint8_t)color.y, (std::uint8_t) color.z, (std::uint8_t) 1));
				}

				point.setNormalIndex(rawPoint.getNormal());
				point.setLidarClassification(rawPoint.getLidarClassification());

				if (rawPoint.getDeleteFlag()) {
					point.setFiltered(true);
					hasDeletedPts = true;
				}
			}

			if (hasDeletedPts) {
				//set flags and for both the voxel and the parent scan
				setHasPeristentDeletedPts(true);
				//m_parentTreePtr->setHasPeristentDeletedPts(true);
			}

			if (m_timeStampOffsetStarts)
			{
				for (int i = 0; i < pointsToBeStreamedIn; i++)
				{
					m_timeStampList[i + oldPoints] = timeStampData[i];
				}
			}

			//applyCrop();
			//applySelections(oldPoints);   
			m_currentLODLoaded = lodLevel;
			m_currentDrawLOD = lodLevel;

			if (!Singleton<AgSceneDatabase>::instance().m_pointCloudManager.getLightWeight())
			{
				updateLayerEffectWhenRefinePoints(oldPoints);

				if (!Singleton<AgSceneDatabase>::instance().m_pointCloudManager.getIgnoreClip())
					updateClipEffectWhenRefinePoints(oldPoints);
			}
		}
	}

	void AgVoxelContainer::getLODInternalLoadInfo(int newLOD, std::uint64_t& fileOffset, int& bytesToRead) const
	{
		fileOffset = 0;
		bytesToRead = 0;

		int amountOfPoints = m_amountOfLODPoints[newLOD];
		if (amountOfPoints <= 0)
			return;

		//only stream in additional points
		int oldPoints = (m_currentLODLoaded < 0) ? 0 : m_amountOfLODPoints[m_currentLODLoaded];

		//new points to be streamed in
		int pointsToBeStreamedIn = amountOfPoints - oldPoints;
		if (pointsToBeStreamedIn <= 0)
			return;

		fileOffset = m_pointDataOffsetStart + (sizeof(AgVoxelLeafNode) * oldPoints);
		bytesToRead = sizeof(AgVoxelLeafNode) * pointsToBeStreamedIn;
	}

	/*void AgVoxelContainer::loadLODInternalFromRawData(int newLOD, void* data, int numBytes)
	{
		AgVoxelLeafNode* rawPoints = static_cast<AgVoxelLeafNode*>(data);

		int amountOfPoints = m_amountOfLODPoints[newLOD];
		int oldPoints = (m_currentLODLoaded < 0) ? 0 : m_amountOfLODPoints[m_currentLODLoaded];
		int pointsToBeStreamedIn = amountOfPoints - oldPoints;

		if (numBytes != pointsToBeStreamedIn * int(sizeof(AgVoxelLeafNode)))
		{
			assert(false, "Mismatched raw data size");
			return;
		}

		bool hasDeletedPts = false;
		if (m_parentTreePtr->isLidarData())
		{
			m_lidarPointList.resize(amountOfPoints);

			//convert to internal format
			RCSpatialReference* pMentorTransformation = m_parentTreePtr->getMentorTransformation();
			for (int i = 0; i < pointsToBeStreamedIn; i++)
			{
				auto& point = m_lidarPointList[i + oldPoints];
				auto& rawPoint = rawPoints[i];

				if (pMentorTransformation != NULL)
				{
					auto coord = VoxelTreeRawPointConverter::rawPointToWorldCoord(this, rawPoint);
					pMentorTransformation->transform(coord);
					point.setRawCoord(VoxelTreeRawPointConverter::worldCoordToRawPoint(this, coord).convertTo<float>());
				}
				else
				{
					point.setRawCoord(rawPoint.getRawOffsetFromBoundingBox());
				}

				if (m_parentTreePtr->hasIntensity())
				{
					point.setRGBA(rawPoint.getRGBA());
				}
				else
				{
					RCVector4ub color = rawPoint.getRGBA();
					point.setRGBA(RCVector4ub((std::uint8_t)color.x, (std::uint8_t)color.y, (std::uint8_t) color.z, (std::uint8_t) 1));
				}

				point.setNormalIndex(rawPoint.getNormal());
				point.setLidarClassification(rawPoint.getLidarClassification());

				if (rawPoint.getDeleteFlag()) {
					point.setFiltered(true);
					hasDeletedPts = true;
				}
			}
		}
		else
		{
			m_terrestialPointList.resize(amountOfPoints);

			//convert to internal format, starting from previous amount of points   
			for (int i = 0; i < pointsToBeStreamedIn; i++)
			{
				auto& point = m_terrestialPointList[i + oldPoints];
				auto& rawPoint = rawPoints[i];
				point.setRawCoord(rawPoint.getRawOffsetFromBoundingBox());

				if (m_parentTreePtr->hasIntensity())
					point.setRGBA(rawPoint.getRGBA());
				else
				{
					RCVector4ub color = rawPoint.getRGBA();
					point.setRGBA(RCVector4ub((std::uint8_t)color.x, (std::uint8_t)color.y, (std::uint8_t) color.z, (std::uint8_t) 1));
				}

				point.setNormalIndex(rawPoint.getNormal());

				if (rawPoint.getDeleteFlag()) {
					point.setFiltered(true);
					hasDeletedPts = true;
				}
			}
		}

		if (hasDeletedPts) {
			//set flags and for both the voxel and the parent scan
			setHasPeristentDeletedPts(true);
			m_parentTreePtr->setHasPeristentDeletedPts(true);
		}

		m_currentLODLoaded = newLOD;
		m_currentDrawLOD = newLOD;

		if (!m_parentTreePtr->getWorld()->getPointCloudProject()->getLightWeight())
		{
			updateLayerEffectWhenRefinePoints(oldPoints);
			updateClipEffectWhenRefinePoints(oldPoints);
		}
	}*/

	//void AgVoxelContainer::loadLidarLODInternalNoSelection(int lodLevel, RCMemoryMapFile* memMapFile  /*= NULL */)
	//{
	//	int amountOfPoints = m_amountOfLODPoints[lodLevel];

	//	std::vector<AgVoxelLeafNode>   rawPoints(amountOfPoints);
	//	m_lidarPointList.resize(amountOfPoints);

	//	if (!memMapFile)
	//	{
	//		RCMemoryMapFile memmap(m_parentTreePtr->getFileName().c_str());
	//		if (memmap.createFileHandleOnlyRead())
	//		{
	//			memMapFile = &memmap;
	//			memMapFile->setFilePointer(m_pointDataOffsetStart);
	//			memMapFile->readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * amountOfPoints);
	//		}
	//	}
	//	else
	//	{
	//		memMapFile->setFilePointer(m_pointDataOffsetStart);
	//		memMapFile->readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * amountOfPoints);
	//	}

	//	//convert to internal format
	//	for (int i = 0; i < amountOfPoints; i++)
	//	{
	//		auto& rawPoint = rawPoints[i];
	//		m_lidarPointList[i].setRawCoord(rawPoint.getRawOffsetFromBoundingBox());
	//		m_lidarPointList[i].setRGBA(rawPoint.getRGBA());
	//		m_lidarPointList[i].setNormalIndex(rawPoint.getNormal());
	//		m_lidarPointList[i].setLidarClassification(rawPoint.getLidarClassification());
	//	}

	//	m_currentLODLoaded = lodLevel;
	//}

	void AgVoxelContainer::loadTerrestialLODInternalNoSelection(int newLOD, RCMemoryMapFile* memMapFile /*= NULL */)
	{

		int amountOfPoints = m_amountOfLODPoints[newLOD];

		std::vector<AgVoxelLeafNode> rawPoints(amountOfPoints);
		m_terrestialPointList.resize(amountOfPoints);

		if (!memMapFile)
		{
			RCMemoryMapFile memmap(m_parentTreePtr->getFileName().c_str());
			if (memmap.createFileHandleOnlyRead())
			{
				memmap.setFilePointer(m_pointDataOffsetStart);
				memmap.readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * amountOfPoints);
			}
		}
		else
		{
			memMapFile->setFilePointer(m_pointDataOffsetStart);
			memMapFile->readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * amountOfPoints);
		}

		//convert to internal format
		for (int i = 0; i < amountOfPoints; i++)
		{
			auto& rawPoint = rawPoints[i];
			m_terrestialPointList[i].setRawCoord(rawPoint.getRawOffsetFromBoundingBox());
			m_terrestialPointList[i].setRGBA(rawPoint.getRGBA());
			m_terrestialPointList[i].setNormalIndex(rawPoint.getNormal());
		}
		m_currentLODLoaded = newLOD;
	}

	/*std::vector<AgVoxelLeafNode>
		AgVoxelContainer::loadLODPointToPointList(int newLOD, RCMemoryMapFile* memMapFile) const
	{
		int pointsToBeStreamedIn = m_amountOfLODPoints[newLOD];
		if (pointsToBeStreamedIn <= 0)
			return std::vector<AgVoxelLeafNode>();

		std::vector<AgVoxelLeafNode> rawPoints(pointsToBeStreamedIn);
		if (!memMapFile)
		{
			RCMemoryMapFile memmap(m_parentTreePtr->getFileName().c_str());

			if (memmap.createFileHandleOnlyRead())
			{
				memmap.setFilePointer(m_pointDataOffsetStart);
				memmap.readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * pointsToBeStreamedIn);
			}
			else {
				rcassert(false, " AgVoxelContainer::loadLODPointToPointList: File Handle Confilcts!\n ");
			}
		}
		else
		{
			long long result = memMapFile->setFilePointer(m_pointDataOffsetStart);
			if (result != -1)
			{

			}
			else
			{
				rcassert(false, " AgVoxelContainer::loadLODPointToPointList: Invalid File Reading!\n");
			}

			bool success = memMapFile->readFile(&rawPoints[0], sizeof(AgVoxelLeafNode) * pointsToBeStreamedIn);
			if (success == true)
			{

			}
			else
			{
				rcassert(false, " AgVoxelContainer::loadLODPointToPointList: Invalid File Reading!\n");
			}
			//const char* dataPtr = memMapFile->mapViewOfFileOnlyRead( m_pointDataOffsetStart, sizeof( VoxelLeafNode ) * pointsToBeStreamedIn );
			//if( !dataPtr )
			//  rcassert( false, "AgVoxelContainer::loadLODPointToPointList" );
			////copy data from starting from previous offset
			//memcpy( &rawPoints[0], dataPtr, sizeof( VoxelLeafNode ) * pointsToBeStreamedIn );
			//memMapFile->unmapViewOfFileOnlyRead( dataPtr );
		}

		return rawPoints;
	}*/

	/*void AgVoxelContainer::loadRawPoints(int pointNumber, RCMemoryMapFile* memMapFile, std::vector<AgVoxelLeafNode>& out) const
	{
		if (pointNumber <= 0 || pointNumber > m_amountOfLODPoints[m_maximumLOD - 1])
		{
			return;
		}
		assert(pointNumber <= m_amountOfLODPoints[m_maximumLOD - 1]);
		out.clear();
		out.resize(pointNumber);
		if (memMapFile)
		{
			long long result = memMapFile->setFilePointer(m_pointDataOffsetStart);
			if (result != -1)
			{

			}
			else
			{
				rcassert(false, " AgVoxelContainer::loadLODPointToPointList: Invalid File Reading!\n");
			}

#ifdef _DEBUG
			const bool success =
#endif
				memMapFile->readFile(&out[0], sizeof(AgVoxelLeafNode) * pointNumber);
			rcassert(success, " AgVoxelContainer::loadRawPoints: Invalid File Reading!\n");
		}
		else
		{
			RCMemoryMapFile memmap(m_parentTreePtr->getFileName().c_str());

			if (memmap.createFileHandleOnlyRead())
			{
				memmap.setFilePointer(m_pointDataOffsetStart);
				memmap.readFile(&out[0], sizeof(AgVoxelLeafNode) * pointNumber);
			}
			else {
				rcassert(false, " AgVoxelContainer::loadRawPoints: File Handle Confilcts!\n ");
			}
			memmap.closeFileHandle();
		}

	}*/
	void AgVoxelContainer::updateClipEffectWhenRefinePoints(size_t pointStartIndex)
	{
		/*if (m_parentTreePtr->getWorld()->getCanDrawWorld() == false)
			return;

		m_parentTreePtr->getWorld()->getPointSelectionManager() ->
			getPointClipManager()->updateClipEffecOnAgVoxelContainer(this, pointStartIndex);*/
	}

	void AgVoxelContainer::updateLayerEffectWhenRefinePoints(size_t pointStartIndex)
	{
		/*if (m_parentTreePtr->getWorld()->getCanDrawWorld() == false)
			return;

		m_parentTreePtr->getWorld()->getPointSelectionManager() ->
			refineSelectionEffectOnAgVoxelContainer(this, pointStartIndex);*/
	}

	void AgVoxelContainer::clearAllViewId()
	{
		m_viewId = 0;
		m_visibleListId = -1;
	}

	void AgVoxelContainer::clearView(int id)
	{
		m_viewId &= ~(1 << id);
	}

	void AgVoxelContainer::setView(int id)
	{
		m_viewId |= (1 << id);
	}

	int AgVoxelContainer::hasView(int id) const
	{
		return ((m_viewId >> id) & 0x01);
	}


	int  AgVoxelContainer::clipIndex() const
	{
		return m_nClipIndex;
	}

	void AgVoxelContainer::setClipIndex(int nClipIndex)
	{
		m_nClipIndex = nClipIndex;
	}

	bool AgVoxelContainer::isClipFlag(ClipFlag rhs) const
	{
		return m_clipFlag == rhs;
	}
	ClipFlag AgVoxelContainer::getClipFlag() const
	{
		return m_clipFlag;
	}

	ClipFlag AgVoxelContainer::getInternalClipFlag() const
	{
		return mInternalClipFlag;
	}

	/*AgEngineSpatialFilter::FilterResult AgVoxelContainer::getInternalClipEngineFilterResult() const
	{
		return clipFlagToEngineFilterResult(mInternalClipFlag);
	}*/

	bool AgVoxelContainer::isInternalClipFlag(ClipFlag rhs) const
	{
		return mInternalClipFlag == rhs;
	}

	void AgVoxelContainer::setExternalClipFlag(ClipFlag rhs)
	{
		mExternalClipFlag = rhs;
		updateClipFlag();
	}
	void AgVoxelContainer::setInternalClipFlag(ClipFlag rhs)
	{
		mInternalClipFlag = rhs;
		updateClipFlag();
	}

	ClipFlag AgVoxelContainer::intersectClipFlag(
		const ClipFlag& lhs,
		const ClipFlag& rhs)
	{
		if (lhs == ALL_CLIPPED || rhs == ALL_CLIPPED)
		{
			return ALL_CLIPPED;
		}
		if (lhs == PARTIAL_CLIPPED || rhs == PARTIAL_CLIPPED)
		{
			return PARTIAL_CLIPPED;
		}
		return NON_CLIPPED;
	}

	void AgVoxelContainer::updateClipFlag()
	{
		m_clipFlag = intersectClipFlag(mInternalClipFlag, mExternalClipFlag);
	}

	void AgVoxelContainer::setRegionFlagInvalid()
	{
		m_regionFlag &= ~(0x1 << 2);
	}
	void AgVoxelContainer::setRegionFlagValid()
	{
		m_regionFlag |= 0x1 << 2;
	}
	bool AgVoxelContainer::isRegionFlagValid() const
	{
		return (m_regionFlag & 0x1 << 2) != 0;
	}
	void AgVoxelContainer::setRegionIndex(std::uint8_t index)
	{
		setRegionFlagValid();
		m_regionFlag &= ~(0x7F << 3);
		m_regionFlag |= index << 3;
	}
	std::uint8_t AgVoxelContainer::getRegionIndex() const
	{
		return static_cast<std::uint8_t>((m_regionFlag >> 3) & 0x7F);
	}

	/*bool AgVoxelContainer::isInheritRegionFlagValid() const
	{
		if (m_parentTreePtr->isRegionFlagValid())
		{
			return true;
		}
		else
		{
			return isRegionFlagValid();
		}
	}

	std::uint8_t AgVoxelContainer::getInheritRegionIndex() const
	{
		if (m_parentTreePtr->isRegionFlagValid())
		{
			return m_parentTreePtr->getRegionIndex();
		}
		else
		{
			//user should check if the region flag of this voxel container is valid or not first
			return getRegionIndex();
		}
	}*/

	void AgVoxelContainer::setInTempSelectionFlagInvalid()
	{
		m_regionFlag &= ~(0x1);
	}
	void AgVoxelContainer::setInTempSelectionFlagValid()
	{
		m_regionFlag |= 0x1;
	}
	bool AgVoxelContainer::isInTempSelectionFlagValid() const
	{
		return m_regionFlag & 0x1;
	}
	void AgVoxelContainer::setIsInTempSelection(bool flag)
	{
		setInTempSelectionFlagValid();
		m_regionFlag &= ~(0x1 << 1);
		m_regionFlag |= flag << 1;
	}

	bool AgVoxelContainer::getIsInTempSelection() const
	{
		return (m_regionFlag >> 1) & 0x1;
	}

	void AgVoxelContainer::setTransformedCenter(const RCVector3d& center)
	{
		m_transformedCenter = center;
	}

	/*RCVector3d AgVoxelContainer::getTransformedCenter() const
	{
		RCVector3d center(m_transformedCenter);
		if (m_parentTreePtr != NULL)
		{
			RCSpatialReference* pTransformation = m_parentTreePtr->getMentorTransformation();
			if (pTransformation != NULL)
			{
				center = m_parentTreePtr->getWorld()->globalToWorld(center);

				pTransformation->transform(center);

				center = m_parentTreePtr->getWorld()->worldToGlobal(center);
			}
		}

		return center;
	}*/
	void AgVoxelContainer::setTransformedSVOBound(const RCBox& rhs)
	{
		m_transformedSVOBounds = rhs;
	}

	RCBox AgVoxelContainer::getTransformedSVOBound() const
	{
		return m_transformedSVOBounds;
	}

	/*RCBox AgVoxelContainer::getMentorTransformedSVOBound() const
	{
		RCBox svoBound = getTransformedSVOBound();

		RCSpatialReference* mentorTrans = m_parentTreePtr->getMentorTransformation();
		if (mentorTrans != NULL && m_parentTreePtr->isLidarData())
		{
			svoBound = m_parentTreePtr->getWorld()->globalToWorld(svoBound);

			svoBound = mentorTrans->transformBounds(svoBound);

			svoBound = m_parentTreePtr->getWorld()->worldToGlobal(svoBound);
		}

		return svoBound;
	}*/

	AgBoundingbox::Handle AgVoxelContainer::getSVOBound() const
	{
		return m_svoBounds;
	}

	void AgVoxelContainer::setTransformedNodeRadius(double rhs)
	{
		m_nodeRadius = rhs;
	}
	double AgVoxelContainer::getTransformedNodeRadius() const
	{
		return m_nodeRadius;
	}

	/*double AgVoxelContainer::getFinestBoundLength()
	{
		double svoBoundLength = m_svoBounds.getMax().x - m_svoBounds.getMin().x;
		double minLeafNodeDist = svoBoundLength / pow(2.0, m_maximumLOD - 1);
		return minLeafNodeDist;
	}

	std::vector<int> AgVoxelContainer::getVisibleNodeIndices()
	{
		RCEngineState* worldPtr = m_parentTreePtr->getWorld();
		alPointSelectionManager *selManger = worldPtr->getPointSelectionManager();
		PointRegionManager*      regionManager = selManger->getPointRegionManager();

		std::vector<int> allVisibleNodeIndices;
		if (!m_parentTreePtr->isLidarData())
		{
			for (size_t i = 0; i < m_terrestialPointList.size(); i++)
			{
				AgVoxelTerrestialPoint terrestialPoint = m_terrestialPointList.at(i);
				// continue if the point is filtered
				if (terrestialPoint.isClipped() || terrestialPoint.isFiltered() || terrestialPoint.isDeleted(this))
				{
					continue;
				}

				std::uint8_t layerIndex = terrestialPoint.getInheritRegionIndex(this);
				if (layerIndex > 0)
				{
					alLayer* layer = regionManager->getLayerAt(layerIndex - 1);
					// continue if the layer is invisible
					if (layer != NULL && !layer->isLayerRemoved() && !layer->getLayerVisible())
					{
						continue;
					}
				}

				allVisibleNodeIndices.push_back((int)i);
			}
		}
		else
		{
			for (size_t i = 0; i < m_lidarPointList.size(); i++)
			{
				AgVoxelLidarPoint lidarPoint = m_lidarPointList.at(i);
				// continue if the point is filtered
				if (lidarPoint.isClipped() || lidarPoint.isFiltered() || lidarPoint.isDeleted(this))
				{
					continue;
				}

				std::uint8_t layerIndex = lidarPoint.getLayerInfo();
				if (layerIndex > 0)
				{
					alLayer* layer = regionManager->getLayerAt(layerIndex - 1);
					// continue if the layer is invisible
					if (layer != NULL && !layer->isLayerRemoved() && !layer->getLayerVisible())
					{
						continue;
					}
				}

				allVisibleNodeIndices.push_back((int)i);
			}
		}

		return allVisibleNodeIndices;
	}*/

	bool AgVoxelContainer::isComplete(int LOD) const
	{
		return m_amountOfLODPoints[LOD] <= m_amountOfLODPoints[m_currentLODLoaded];
	}
}