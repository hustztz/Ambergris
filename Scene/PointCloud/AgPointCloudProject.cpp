#include "AgPointCloudProject.h"
#include "AgVoxelContainer.h"
#include "AgRegionVisibility.h"
#include "Resource/AgRenderResourceManager.h"

#include <utility/RCTimer.h>
#include <utility/RCStringUtils.h>
#include <algorithm>

using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility::String;

namespace ambergris {

	static bool calcLODs(AgBoundingbox::Handle voxelBounds, AgVoxelTreeRunTime::Handle voxelTree, std::vector< LodRecord >& lodsOut)
	{
		const uint16_t views_size = (uint16_t)Singleton<AgRenderResourceManager>::instance().m_views.getSize();
		lodsOut.resize(views_size);
		bool someGood = false;
		for (uint16_t i = 0; i != views_size; ++i)
		{
			const AgCameraView* view = Singleton<AgRenderResourceManager>::instance().m_views.get(i);
			if(!view || AgCameraView::kInvalidHandle == view->m_handle)
				continue;
			const int lod = view->calcLOD(voxelBounds, voxelTree);
			someGood |= (lod > 0);
			lodsOut[i].m_LOD = (std::uint8_t) lod;
		}
		return someGood;
	}

	static bool calcVoxelLODs(const AgVoxelContainer& voxel, std::vector< LodRecord >& lodsOut)
	{
		AgBoundingbox* svoBbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(voxel.m_svoBounds);
		if (!svoBbox)
			return false;

		const AgBoundingbox* nodeBbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(voxel.m_nodeBounds);
		if (!nodeBbox)
			return false;

		const double svoBoundExtent = (double)svoBbox->getRadius();
		const double nodeBoundExtent = (double)nodeBbox->getRadius();
		if (svoBoundExtent < RealityComputing::Common::Math::Constants::epsilon)
			return false;

		const double ratio = nodeBoundExtent / svoBoundExtent;

		if (ratio > 0.5)
		{
			//use the original svo bound

			if (calcLODs(voxel.m_svoBounds, voxel.getParentTreeHandle(), lodsOut) == false)
				return false;

			for (auto& lod : lodsOut)
			{
				lod.m_LOD = std::min(lod.m_LOD, (uint8_t)(32 - 1));
				lod.m_desiredPointCount = voxel.m_amountOfLODPoints[lod.m_LOD];
			}
		}
		else
		{
			//refine the voxel container to reach the level of node bound
			const double log2e = 1.44269504088896340736; //log2(e)
			const uint8_t refineLevel = static_cast<uint8_t>(std::log(ratio) * log2e *-1.0);

			//use node bound to compute LOD and then increase by refineLevel to estimate level w.r.t svoBound
			double radius = (double)svoBbox->getRadius() * 0.5;

			//in case of there is only one point inside the voxel, the radius will be zero
			if (radius < Math::Constants::epsilon)
				svoBbox->expand((float)(radius - Math::Constants::epsilon));

			if (calcLODs(voxel.m_svoBounds, voxel.getParentTreeHandle(), lodsOut) == false)
				return false;

			for (auto& lod : lodsOut)
			{
				lod.m_LOD += refineLevel;
				lod.m_LOD = std::min(lod.m_LOD, (uint8_t)(32 - 1));
				lod.m_desiredPointCount = voxel.m_amountOfLODPoints[lod.m_LOD];
			}
		}

		return true;
	}

	static inline LodRecord getLOD(const ScanContainerID& container, int viewIndex)
	{
		if (static_cast<int>(container.m_LODs.size()) > viewIndex)
			return container.m_LODs[viewIndex];
		return LodRecord();
	}

	AgPointCloudProject::AgPointCloudProject()
		: mMaxPointsLoad(75)
		, mLightWeight(false)
		, mIsProjectDirty(false)
		, mCoordinateSystemHasChanged(false)
	{
		m_geoReference[0] = m_geoReference[1] = m_geoReference[2] = 0.0;
	}

	int AgPointCloudProject::reducePointCloudLoad(std::uint32_t amountOfPointsVisible, AgCameraView::Handle view, std::vector<ScanContainerID>& voxelContainerListOut)
	{
		if (AgCameraView::kInvalidHandle == view)
			return 0;

		auto totalVisiblePointsNew = amountOfPointsVisible;
		//int reductionFactor = 0;                      //in LOD levels
		auto amountOfPointsMax = mMaxPointsLoad * 1000000;
		while (totalVisiblePointsNew > amountOfPointsMax)
		{
			totalVisiblePointsNew = 0;
			for (size_t i = 0; i < voxelContainerListOut.size(); i++)
			{
				ScanContainerID& contId = voxelContainerListOut[i];
				AgVoxelContainer* containerPTr = m_scanList[contId.m_scanId]->getVoxelContainerAt(contId.m_containerId);
				std::uint8_t& lodLevel = contId.m_LODs[view].m_LOD; //fetch lod
				std::uint32_t& lodCount = contId.m_LODs[view].m_pointCount;
				if (lodLevel > 0)
					lodLevel--;

				totalVisiblePointsNew += containerPTr->m_amountOfLODPoints[lodLevel];

				if (lodLevel <= containerPTr->m_currentLODLoaded)
					lodCount = containerPTr->m_amountOfLODPoints[lodLevel];
				else if (containerPTr->m_currentLODLoaded > 0)
					lodCount = containerPTr->m_amountOfLODPoints[containerPTr->m_currentLODLoaded];
				else
					lodCount = 0;
			}
		}
		return totalVisiblePointsNew;
	}

	std::uint32_t AgPointCloudProject::getTotalPointCount(AgCameraView::Handle view, const std::vector<ScanContainerID>& voxelContainerListOut) const
	{
		std::uint32_t numPoints = 0;

		for (size_t i = 0; i < voxelContainerListOut.size(); i++)
		{
			const ScanContainerID& contId = voxelContainerListOut[i];
			AgVoxelContainer* containerPTr = m_scanList[contId.m_scanId]->getVoxelContainerAt(contId.m_containerId);
			std::uint8_t lod = getLOD(contId, view).m_LOD;
			lod = std::min(lod, (uint8_t)(32 - 1));
			numPoints += containerPTr->m_amountOfLODPoints[lod];
		}
		return numPoints;
	}

	int AgPointCloudProject::_ReducePointCloudLoad(std::uint32_t amountOfPointsVisible, AgCameraView::Handle view, std::vector<ScanContainerID>& voxelContainerListOut)
	{
		if (AgCameraView::kInvalidHandle == view)
			return 0;

		auto totalVisiblePointsNew = amountOfPointsVisible;
		//int reductionFactor = 0;                      //in LOD levels
		auto amountOfPointsMax = mMaxPointsLoad * 1000000;
		while (totalVisiblePointsNew > amountOfPointsMax)
		{
			totalVisiblePointsNew = 0;
			for (size_t i = 0; i < voxelContainerListOut.size(); i++)
			{
				ScanContainerID& contId = voxelContainerListOut[i];
				AgVoxelContainer* containerPTr = m_scanList[contId.m_scanId]->getVoxelContainerAt(contId.m_containerId);
				std::uint8_t& lodLevel = contId.m_LODs[view].m_LOD; //fetch lod
				std::uint32_t& lodCount = contId.m_LODs[view].m_pointCount;
				if (lodLevel > 0)
					lodLevel--;

				totalVisiblePointsNew += containerPTr->m_amountOfLODPoints[lodLevel];

				if (lodLevel <= containerPTr->m_currentLODLoaded)
					lodCount = containerPTr->m_amountOfLODPoints[lodLevel];
				else if (containerPTr->m_currentLODLoaded > 0)
					lodCount = containerPTr->m_amountOfLODPoints[containerPTr->m_currentLODLoaded];
				else
					lodCount = 0;
			}
		}
		return totalVisiblePointsNew;
	}

	std::wstring AgPointCloudProject::getProjectOriginalCoordinateSystem() const
	{
		std::wstring cs = L"";

		size_t scansCount = m_scanList.size();
		if (scansCount == 0)
		{
			return cs;
		}

		for (size_t i = 0; i < scansCount; i++)
		{
			AgVoxelTreeRunTime* runtimeTree = m_scanList.at(i);
			if (runtimeTree != NULL)
			{
				const std::wstring coordinateSys = runtimeTree->getOriginalCoordinateSystem();
				if (i == 0)
				{
					cs = coordinateSys;
				}
				else
				{
					if (!RCStringUtils::wequal(cs, coordinateSys, true))
					{
						cs = L"";
						return cs;
					}
				}
			}
		}

		return cs;
	}

	std::wstring AgPointCloudProject::getProjectCurrentCoordinateSystem() const
	{
		std::wstring cs = L"";

		size_t scansCount = m_scanList.size();
		if (scansCount == 0)
		{
			return cs;
		}

		for (size_t i = 0; i < scansCount; i++)
		{
			AgVoxelTreeRunTime* runtimeTree = m_scanList.at(i);
			if (runtimeTree != NULL)
			{
				const std::wstring coordinateSys = runtimeTree->getTargetCoordinateSystem();
				if (i == 0)
				{
					cs = coordinateSys;
				}
				else
				{
					if (!RCStringUtils::wequal(cs, coordinateSys, true))
					{
						cs = L"";
						return cs;
					}
				}
			}
		}

		return cs;
	}

	void AgPointCloudProject::updateProjectBounds()
	{
		m_visibleProjectBounds.clear();
		m_projectBounds.clear();
		for (const auto& s : m_scanList)
		{
			const auto scanBounds = s->getTransformedBounds();
			auto boundsLength = scanBounds.getMax().distanceSqrd(scanBounds.getMin());
			if (isinf(boundsLength) || isnan(boundsLength))
				continue;
			if (s->isVisible())
			{
				m_visibleProjectBounds.updateBounds(scanBounds);
			}
			m_projectBounds.updateBounds(scanBounds);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// \brief: set to global from world transform
	//////////////////////////////////////////////////////////////////////////
	void AgPointCloudProject::setToGlobalFromWorldTransform(const RCTransform& toGlobalFromWorld)
	{
		mToGlobalFromWorld = toGlobalFromWorld;

		for (size_t i = 0; i < m_scanList.size(); i++)
		{
			AgVoxelTreeRunTime* curTreePtr = m_scanList[i];
			curTreePtr->setAdditionalTransform(mToGlobalFromWorld);

			curTreePtr->updateAll();
		}

		updateProjectBounds();
		updateSpatialFilters();
	}

	//////////////////////////////////////////////////////////////////////////
	// \brief: get to global from world transform
	//////////////////////////////////////////////////////////////////////////
	RCTransform AgPointCloudProject::getToGlobalFromWorldTransform() const
	{
		return mToGlobalFromWorld;
	}

	bool AgPointCloudProject::addScan(AgVoxelTreeRunTime* treePtr)
	{
		if (!treePtr)
		{
			return false;
		}

		// check if the voxel tree is already added, duplicate voxel trees are not allowed
		for (size_t i = 0; i < m_scanList.size(); i++)
		{
			if (m_scanList.at(i)->getId() == treePtr->getId())
			{
				return false;
			}
		}

		//set to current coordinate system
		if (mCoordinateSystemHasChanged)
		{
			const auto currentCS = getProjectCurrentCoordinateSystem();
			if (!currentCS.empty())
			{
				treePtr->setTargetCoordinateSystem(currentCS, true);
			}
		}

		//if geoReference is set for this project, we need to also set geoReference for this new scan
		if (m_geoReference[0] > 1.0e-6 || m_geoReference[1] > 1.0e-6 || m_geoReference[2] > 1.0e-6)
		{
			treePtr->setGeoReference(m_geoReference);
			double pos[3];
			pos[0] = treePtr->getOriginalTranslation()[0] - m_geoReference[0];
			pos[1] = treePtr->getOriginalTranslation()[1] - m_geoReference[1];
			pos[2] = treePtr->getOriginalTranslation()[2] - m_geoReference[2];
			treePtr->setGlobalPosition(pos);
		}

		// Apply any existing UCS.
		treePtr->setAdditionalTransform(mToGlobalFromWorld);
		treePtr->updateAll();

		m_worldPtr->finishPointLoading();
		m_worldPtr->setWorldDirty(true);

		m_scanList.push_back(treePtr);
		updateProjectBounds();

		// For each filter, create a transformed version for this new tree
		RCTransform transInv = treePtr->getTransform().getInverse(TransformType::Rigid);
		for (FilterMap::iterator iFilt = m_filters.begin(); iFilt != m_filters.end(); ++iFilt)
		{
			Interface::ARCSpatialFilter* filterTrans = iFilt->first->transformFilter(transInv);
			iFilt->second.push_back(filterTrans);
			treePtr->addSpatialFilter(filterTrans);
		}

		alLimitBox *limBox = m_worldPtr->getActiveLimitBox();
		if (limBox)
		{
			const std::wstring limBoxName = limBox->getNodeName();

			limBox->fromBounds(m_worldPtr->getExpandedBoundingBox());
			if (limBox->getFirstTime())
			{
				limBox->setFirstTime(false);
			}
			else
			{
				// Because that LimitBox::fromBounds sets the node name to "Default",
				// restore its name after the fromBounds call.
				limBox->setNodeName(limBoxName);
			}
		}


		mIsProjectDirty = true;

		return true;
	}

	bool AgPointCloudProject::unloadProject()
	{
		for (auto& s : m_scanList)
		{
			delete s;
		}
		m_scanList.clear();

		//Do not delete the render target, just de-alloc it's members
		if (mOccluderTargetPtr)
			mOccluderTargetPtr->Clear();

		m_geoReference[0] = 0.0;
		m_geoReference[1] = 0.0;
		m_geoReference[2] = 0.0;
		m_visibleProjectBounds.clear();
		m_projectBounds.clear();

		// Nuke all the transformed filters that we have created for each scan.
		for (FilterMap::iterator iFilter = m_filters.begin(); iFilter != m_filters.end(); ++iFilter)
		{
			for (size_t iScan = 0; iScan != iFilter->second.size(); ++iScan)
			{
				delete iFilter->second[iScan];
			}
			iFilter->second.clear();
		}

		mCoordinateSystemHasChanged = false;
		mIsProjectDirty = false;

		// set project unit to meter
		RCUnitService::setUnitType(UnitType::UTMeter);

		clearDebugBounds();
		clearDebugPoints();
		clearDebugTriangles();
		clearDebugQuads();

		return true;
	}

	//visible voxels after frustum culling
	void    AgPointCloudProject::_GetVisibleNodesFromScan(int scan, std::vector<ScanContainerID>& voxelContainerListOut)
	{
		const double curTime = RealityComputing::Utility::Time::getTimeSinceEpochinMilliSec();
		int treeIndex = scan;
		AgVoxelTreeRunTime* curTreePtr = m_scanList[treeIndex];

		// Allocate an element into the list that will become the next working element.
		voxelContainerListOut.reserve(voxelContainerListOut.size() + curTreePtr->getAmountOfVoxelContainers() + 1);
		voxelContainerListOut.resize(voxelContainerListOut.size() + 1);

		//for each voxel container
		for (int j = 0; j < curTreePtr->getAmountOfVoxelContainers(); j++)
		{
			AgVoxelContainer* renderLeaf = curTreePtr->getVoxelContainerAt(j);

			// xuxi: if the voxel is clipped, skip it
			if (renderLeaf->isClipFlag(ALL_CLIPPED))
				continue;

			if (!mLightWeight)
			{
				// See if the voxel is in a visible region
				if (!AgRegionVisibility::isVoxelRegionVisible(*renderLeaf))
					continue;
			}

			// See if its LOD is not zero
			ScanContainerID& containerId = voxelContainerListOut.back();
			if (calcVoxelLODs(*renderLeaf, containerId.m_LODs))
			{
				// do not test for spatial filters here, which has been done by doCropFilterForScanAndVoxelLevel
				if (!renderLeaf->isClipFlag(ALL_CLIPPED))
				{
					renderLeaf->m_lastTimeModified = curTime;
					renderLeaf->m_visibleListId = (int)voxelContainerListOut.size();

					containerId.m_scanId = treeIndex;
					containerId.m_containerId = j;

					//use flip flag on the voxel container to set spatial filter result
					switch (renderLeaf->getClipFlag())
					{
					case ALL_CLIPPED:
						containerId.m_spatialFilterResult = FILTER_OUTSIDE;
						break;
					case PARTIAL_CLIPPED:
						containerId.m_spatialFilterResult = FILTER_INTERSECTS;
						break;
					case NON_CLIPPED:
						containerId.m_spatialFilterResult = FILTER_INSIDE;
						break;
					}

					//update num points
					for (size_t k = 0; k != containerId.m_LODs.size(); ++k)
					{
						auto& lodLevel = containerId.m_LODs[k].m_LOD;
						auto& lodCount = containerId.m_LODs[k].m_pointCount;

						if (lodLevel <= renderLeaf->m_currentLODLoaded)
							lodCount = renderLeaf->m_amountOfLODPoints[lodLevel];
						else if (renderLeaf->m_currentLODLoaded > 0)   //needs refinement reset to zero
							lodCount = renderLeaf->m_amountOfLODPoints[renderLeaf->m_currentLODLoaded];
						else
							lodCount = 0;
					}
					//add this to result list by creating a new working scan ID
					voxelContainerListOut.resize(voxelContainerListOut.size() + 1);
				}
				else
					continue;
			}
			else
			{
				continue;
			}
		}
		// remove the last element, since it was just a working value
		voxelContainerListOut.pop_back();
	}

	int AgPointCloudProject::_GetVisibleNodesFromScans(const std::vector<PointCloudInformation> &visibleScanList,
		std::vector<ScanContainerID>& voxelContainerListOut)
	{
		//TODO: Consider if we can replace PointCloudInformation with int
		int potentialVisiblenodes = 0;
		for (size_t i = 0; i < visibleScanList.size(); i++)
		{
			int treeIndex = visibleScanList[i].m_scanId;
			AgVoxelTreeRunTime* curTreePtr = m_scanList[treeIndex];
			potentialVisiblenodes += curTreePtr->getAmountOfVoxelContainers();
		}
		//reserve already space, for huge point clouds with many nodes, this reduces culling
		//in about half of the time
		if (potentialVisiblenodes)
			voxelContainerListOut.reserve(potentialVisiblenodes);

		for (size_t i = 0; i < visibleScanList.size(); i++)
		{
			_GetVisibleNodesFromScan(visibleScanList[i].m_scanId, voxelContainerListOut);
		}
		//reduce amount of points needed to be loaded
		if (mMaxPointsLoad > 0)
		{
			for (int ii = 0; ii < Singleton<AgRenderResourceManager>::instance().m_views.getSize(); ii ++)
			{
				const AgCameraView* view = Singleton<AgRenderResourceManager>::instance().m_views.get(ii);
				if(!view || AgCameraView::kInvalidHandle == view->m_handle)
					continue;
				std::uint32_t totalPoints = getTotalPointCount(view->m_handle, voxelContainerListOut);
				if (totalPoints > mMaxPointsLoad * 1000000)
				{
					_ReducePointCloudLoad(totalPoints, view->m_handle, voxelContainerListOut);
				}
			}
		}

		return int(voxelContainerListOut.size());
	}

	int AgPointCloudProject::_DoPerformFrustumCulling(std::vector<PointCloudInformation> &visibleTreeListOut)
	{
		std::vector< LodRecord > lodRecs;

		for (size_t i = 0; i < m_scanList.size(); i++)
		{
			AgVoxelTreeRunTime *curTreePtr = m_scanList[i];
			if (!curTreePtr->m_isVisible)
				continue;

			if (curTreePtr->isClipFlag(ALL_CLIPPED))
				continue;

			//skip this when in lightWeight mode
			if (!mLightWeight)
			{
				//check if this tree is deleted or in an invisible region
				if (curTreePtr->isRegionFlagValid() && AgRegionVisibility::isRegionVisible(curTreePtr->getRegionIndex()) == false)
				{
					continue;
				}
			}

			//ToDo: testLods is enough for this purpose?
			if (calcLODs(curTreePtr->getSvoBounds(), curTreePtr->m_handle, lodRecs))      // !! was: getGlobalTransformedBounds()
			{
				// do not test filter here, which has been done by doCropFilterForScanAndVoxelLevel
				//ToDo: Consider if we can use scanId to replace PointCloudInformation directly
				PointCloudInformation scanInfo;
				scanInfo.m_scanId = (int)i;

				visibleTreeListOut.push_back(scanInfo);
			}
		}
		return (int)visibleTreeListOut.size();
	}

	void AgPointCloudProject::doFrustumCullingAndLODDetermination(std::vector<ScanContainerID>& idsOut)
	{
		std::vector<PointCloudInformation> visibleList;

		if (_DoPerformFrustumCulling(visibleList))
		{
			//only perform frustum culling
			if (mCullMethod == kCullMethodFrustumCullingOnly)
			{
				_GetVisibleNodesFromScans(visibleList, idsOut);
			}
			else if (mCullMethod == kCullMethodOccludersOnly)
			{
				//assumes occluder target already has textures --> ( buildOccluders() )
				/*if (mOccluderTargetPtr)
				{
					cullUsingOccluderData(visibleList, idsOut);
				}*/
			}
		}
	}
}