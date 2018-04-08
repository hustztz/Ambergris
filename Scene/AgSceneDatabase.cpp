#include "AgSceneDatabase.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgProjectFileExtensions.h"
#include "Resource\AgRenderResourceManager.h"

#include <common/RCString.h>
#include <utility/RCStringUtils.h>
#include <utility/RCLog.h>
#include <utility/RCFilesystem.h>

#include "BGFX/entry/entry.h"//TODO

using namespace ambergris;
using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility;
using namespace ambergris::RealityComputing::Utility::String;
using namespace ambergris::RealityComputing::Utility::Log;

namespace
{
	void updateLODS(AgPointCloudProject* prj, std::vector<ScanContainerID>& visibleNodes)
	{
		for (size_t j = 0; j < visibleNodes.size(); ++j)
		{
			ScanContainerID& testContainer = visibleNodes[j];
			const AgVoxelTreeRunTime* currTree = prj->get(testContainer.m_scanId);
			if (!currTree || AgVoxelTreeRunTime::kInvalidHandle == currTree->m_handle)
				continue;

			const AgVoxelContainer* renderLeaf = currTree->getVoxelContainerAt(testContainer.m_containerId);

			//update num points
			for (size_t k = 0; k != testContainer.m_LODs.size(); ++k)
			{
				std::uint8_t& lodLevel = testContainer.m_LODs[k].m_LOD;
				std::uint32_t& lodCount = testContainer.m_LODs[k].m_pointCount;

				if (lodLevel <= renderLeaf->m_currentLODLoaded)
					lodCount = renderLeaf->m_amountOfLODPoints[lodLevel];
				else if (renderLeaf->m_currentLODLoaded > 0)   //needs refinement reset to zero
					lodCount = renderLeaf->m_amountOfLODPoints[renderLeaf->m_currentLODLoaded];
				else
					lodCount = 0;
			}
		}
	}
}

AgObject::Handle AgSceneDatabase::appendObject(std::shared_ptr<AgObject> object)
{
	AgObject::Handle handle = m_objectManager.append(object);
	if (AgObject::kInvalidHandle == handle)
		return handle;

	std::shared_ptr<AgMesh> pMesh = std::dynamic_pointer_cast<AgMesh>(object);
	if (pMesh)
	{
		uint32_t selectID = pMesh->m_pick_id[0] + (pMesh->m_pick_id[1] << 8) + (pMesh->m_pick_id[2] << 16) + (255u << 24);
		if (m_select_id_map.find(selectID) == m_select_id_map.end())
		{
#ifdef USING_TINYSTL
			m_select_id_map.insert(stl::make_pair<uint32_t, AgObject::Handle>(selectID, pMesh->m_handle));
#else
			m_select_id_map[selectID] = pMesh->m_handle;
#endif
		}
		else
		{
			// Re-hash
			pMesh->m_pick_id[0] = (pMesh->m_pick_id[0] + 1) % 256;
			pMesh->m_pick_id[1] = (pMesh->m_pick_id[1] + 1) % 256;
			pMesh->m_pick_id[2] = (pMesh->m_pick_id[2] + 1) % 256;
			selectID = pMesh->m_pick_id[0] + (pMesh->m_pick_id[1] << 8) + (pMesh->m_pick_id[2] << 16) + (255u << 24);
#ifdef USING_TINYSTL
			m_select_id_map.insert(stl::make_pair<uint32_t, AgObject::Handle>(selectID, pMesh->m_handle));
#else
			m_select_id_map[selectID] = pMesh->m_handle;
#endif		
		}
	}

	return handle;
}

AgVoxelTreeRunTime::Handle AgSceneDatabase::loadPointCloudFile(const std::wstring& filename, RCCode& returnCode, const PointCloudLoadOptions& opts /*= PointCloudLoadOptions()*/)
{
	if (!Filesystem::exists(filename))
	{
		return AgVoxelTreeRunTime::kInvalidHandle;
	}

	if (Filesystem::lowercaseExtension(filename) == SCAN_FILE_EXTENSION)
	{
		std::wstring errCode;
		std::shared_ptr<AgVoxelTreeRunTime> newTreeFormat(new AgVoxelTreeRunTime());
		newTreeFormat->setUseFileHeaderTransform(opts.useFileHeaderTransform);
		returnCode = newTreeFormat->loadFromMedium(filename, m_projectFile);
		if (returnCode == rcOK || returnCode == rcLegacyFileFormat)
		{
			if (m_pointCloudProject.addScan(newTreeFormat))
			{
				//update bounding boxes
				const AgBoundingbox* bbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(newTreeFormat->m_bbox);
				if (bbox)
				{
					m_worldBounds.updateBounds(bbox->m_bounds);
					mTransformedWorldBounds.updateBounds(newTreeFormat->getTransformedBounds());

					//updateFarPlane();
				}

				//getPointSelectionManager()->getPointClipManager()->applyClipNodeListEffect(newTreeFormat);

				// open file stream so that file can't be renamed or moved
				/*std::wstring fileId = newTreeFormat->getId();
				openFileStream(fileId, filename);*/

				return newTreeFormat->m_handle;
			}
		}
	}

	return AgVoxelTreeRunTime::kInvalidHandle;
}


bool AgSceneDatabase::unloadPointCloudFile(const std::wstring& fileId)
{
	//finishPointLoading();
	//setWorldDirty(true);

	// traverse available trees and unload the given file
	for (uint16_t ii = 0; ii < m_pointCloudProject.getSize(); ii++)
	{
		const AgVoxelTreeRunTime* treeIte = m_pointCloudProject.get(ii);
		if (!treeIte || AgVoxelTreeRunTime::kInvalidHandle == treeIte->m_handle)
			continue;

		// get file id of VoxelTreeRunTime
		if (treeIte->getId() == fileId)
		{
			m_pointCloudProject.removeScan(fileId);
			// close the file stream
			//closeFileStream(fileId);
			return true;
		}
	}

	return false;
}

RCCode AgSceneDatabase::loadProject(std::wstring fileName, bool autoSetGeoRef /*= false*/)
{
	if (fileName.empty() || Filesystem::lowercaseExtension(fileName) != PROJECT_FILE_EXTENSION)
	{
		return rcFalseArgument;
	}

	if (!Filesystem::exists(fileName))
	{
		return rcFileNotExist;
	}

	//already loaded, filename is the same
	if (fileName == m_projectFile)
	{
		return rcOK;
	}
	m_projectFile = fileName;

	//unload current project
	unLoadProject();

	/*UnitType unitType;
	int unitPrecision;
	bool isUnitAdapted;
	infoMgr.getUnitInfo(unitType, unitPrecision, isUnitAdapted);
	RCUnitService::setUnitType(unitType);
	RCUnitService::setUnitPrecision(unitPrecision);
	RCUnitService::setUnitAdapted(isUnitAdapted);*/

	RCCode returnCode = rcOK;

	//load voxelTreeNodes if not already loaded
	//returnCode = checkScanExistence(projectIO);

	// load voxelTree
	//for (size_t i = 0; i < mVoxelTreeNodes.size(); i++)
	//{
	//	std::shared_ptr<AgVoxelTreeRunTime> newTreePtr(new AgVoxelTreeRunTime());
	//	newTreePtr->setUseFileHeaderTransform(false);
	//	newTreePtr->setProjectDirectory(m_projectFile);
	//	if (newTreePtr->onLoad(m_projectFile, mVoxelTreeNodes.at(i), m_pointCloudProject.getLightWeight()))
	//	{
	//		std::wstring fileName = newTreePtr->getFileName();
	//		// check file format
	//		RCCode rsCode = newTreePtr->checkFileFormat(fileName);
	//		if (rsCode != rcOK && returnCode != rcFileNotExist)
	//		{
	//			returnCode = rsCode;
	//		}

	//		m_pointCloudProject.addScan(newTreePtr);
	//		// open file stream so that file can't be renamed or moved
	//		/*std::wstring fileId = newTreePtr->getId();
	//		openFileStream(fileId, fileName);*/
	//	}
	//	else
	//	{
	//		returnCode = rcFileNotExist;
	//	}
	//}

	//update the project w.r.t globalOffset only after all the information has been loaded
	// setGlobalOffset
	/*RCVector3d offset;
	if (projectIO.getGlobalOffset(offset) == rcOK) {
	setGeoReference(offset);
	}*/

	m_pointCloudProject.updateProjectBounds();

	if (autoSetGeoRef)
	{
		autoSetGeoReference();
	}

	return returnCode;
}

bool AgSceneDatabase::unLoadProject()
{
	mVisibleNodes.clear();
	m_pointCloudProject.unloadProject();

	m_worldBounds.clear();
	mTransformedWorldBounds.clear();

	m_projectFile = L"";

	return true;
}

void AgSceneDatabase::_UpdateBounds()
{
	m_pointCloudProject.updateProjectBounds();

	m_worldBounds.clear();
	mTransformedWorldBounds.clear();

	//update bound by all scans
	for (uint16_t i = 0; i < m_pointCloudProject.getSize(); ++i)
	{
		const AgVoxelTreeRunTime* treeIte = m_pointCloudProject.get(i);
		if (!treeIte || AgVoxelTreeRunTime::kInvalidHandle == treeIte->m_handle)
			continue;

		const AgBoundingbox* bbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(treeIte->m_bbox);
		if (bbox)
		{
			m_worldBounds.updateBounds(bbox->m_bounds);
			mTransformedWorldBounds.updateBounds(treeIte->getTransformedBounds());
		}
	}

	/*for (int i = 0; i < mSceneManager->getSize(); i++)
	{
	m_worldBounds.updateBounds(mSceneManager->get(i)->getBoundingBox());

	updateTransformedWorldBounds(mSceneManager->get(i));
	}*/
}

void AgSceneDatabase::_SetPointRequestsForVisibleNodes()
{
	if (!mPointCloudRequests)
		mPointCloudRequests.reset(new AgPointCloudRequests());
	mPointCloudRequests->Clear();
	mPointCloudRequests->SetWorkingSet(mVisibleNodes);
	mPointCloudRequests->Start();
}

void AgSceneDatabase::_UpdateVisibleNodes(AgCameraView::Handle view)
{
	if (true/*mustVisibleNodesBeFullyUpdated()*/)
	{
		mVisibleNodes.clear();
		m_pointCloudProject.doFrustumCullingAndLODDetermination(mVisibleNodes);
		int numberOfVisibleNodes = static_cast<int>(mVisibleNodes.size());
		m_totalPointCount = m_pointCloudProject.getTotalPointCount(view, mVisibleNodes);
		int nTotalPointCountAfterReduction = m_pointCloudProject.reducePointCloudLoad(m_totalPointCount, view, mVisibleNodes);
		_SetPointRequestsForVisibleNodes();
	}
	else
	{
		//No frustum culling, but update LODS's since new data might have loaded
		updateLODS(&m_pointCloudProject, mVisibleNodes);
	}
}

bool AgSceneDatabase::hasGeoReference()
{
	return abs(Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.getGeoReference().length()) > Math::Constants::epsilon;
}

void AgSceneDatabase::setGeoReference(const RCVector3d& offset)
{
	//if the offset is same to the old one
	//do nothing
	RCVector3d oldGeoReference = Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.getGeoReference();
	if (fabs(oldGeoReference.distance(offset)) < Math::Constants::distanceTolerance)
		return;

	Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.setGeoReference(offset);

	m_pointCloudProject.setGeoReference(offset);

	/*for (int i = 0; i < mSceneManager->getSize(); i++)
	{
	mSceneManager->get(i)->updateAll();
	}*/

	_UpdateBounds();
}

void AgSceneDatabase::autoSetGeoReference()
{
	_UpdateBounds();
	//automatically set global offset
	const RCBox& projectBound = m_pointCloudProject.getFullBoundingBox();
	const RCVector3d& projBoundCenter = projectBound.getCenter();
	const bool projectBoundsFarFromOrigin = ((fabs(projBoundCenter.x) > 10000.0 || fabs(projBoundCenter.y) > 10000.0 || fabs(projBoundCenter.z) > 10000.0));
	if (!hasGeoReference() && projectBoundsFarFromOrigin)
	{
		RCVector3d globalOffset(0.0, 0.0, 0.0);
		globalOffset = projectBound.getCenter();

		setGeoReference(globalOffset);

		std::wstring geoRefStr;
		geoRefStr = RCStringUtils::format(L"Geo-Reference: %f, %f, %f", globalOffset.x, globalOffset.y, globalOffset.z);
		RCLog::getLogFile()->addMessage(RCString(geoRefStr.c_str()));
	}
}