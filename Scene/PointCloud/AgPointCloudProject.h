#pragma once

#include "AgVoxelTreeRunTime.h"
#include "Resource/AgResourcePool.h"
#include "Resource/AgCameraView.h"

#include <common/RCTransform.h>

namespace ambergris {

	//////////////////////////////////////////////////////////////////////////
	//\brief:  LodRecord Tells which LOD a container has, and what its size is
	//////////////////////////////////////////////////////////////////////////
	struct LodRecord
	{
		std::uint8_t m_LOD;
		std::uint32_t m_pointCount;
		std::uint32_t m_renderPointCount;
		std::uint32_t m_desiredPointCount;
		LodRecord() : m_LOD(0), m_pointCount(0), m_renderPointCount(0), m_desiredPointCount(0) {}
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	struct ScanContainerID
	{
		int                                     m_scanId;
		int                                     m_containerId;
		std::vector<LodRecord>                  m_LODs;
		int                                     m_spatialFilterResult;
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	struct PointCloudInformation
	{
		int                                     m_scanId;                       //scan id into PointCloudProject::m_scanList
		int                                     m_spatialFilterResult;
	};

	class AgPointCloudProject : public AgResourcePool<AgVoxelTreeRunTime>
	{
		/// <description>
		/// Represents whether a given voxel is inside, outside, or in the edge of a filter.
		/// </description>
		enum FilterResult { FILTER_INSIDE = 0, FILTER_OUTSIDE, FILTER_INTERSECTS };
	public:

		enum POINTCLOUD_CULL_METHOD
		{
			kCullMethodFrustumCullingOnly = 0,
			kCullMethodOccludersOnly = 1,
			kCullMethodHybrid = 2       // TODO
		};

		enum POINTCLOUD_SELECTION_LEVEL
		{
			kTestProject = 0,
			kTestScan = 1,
			kTestVoxel = 2,
			kTestPoint = 3
		};

	public:
		AgPointCloudProject();
		//////////////////////////////////////////////////////////////////////////
		//\brief: Do first a frustum cull with the scans's bounding box,
		//        after that determine the LOD's for each of the voxel containers
		//////////////////////////////////////////////////////////////////////////
		void		doFrustumCullingAndLODDetermination(std::vector<ScanContainerID>& idsOut);

		//////////////////////////////////////////////////////////////////////////
		//\ brief: Returns the total amount of points visible, for this view
		//////////////////////////////////////////////////////////////////////////
		std::uint32_t      getTotalPointCount(AgCameraView::Handle view, const std::vector<ScanContainerID>& voxelContainerListOut) const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: Reduce the amount of points that are streamed in, until it
		// falls below 'mMaxPointsLoad' returns the new number of
		// points that will be streamed in
		//////////////////////////////////////////////////////////////////////////
		int                reducePointCloudLoad(std::uint32_t amountOfPointsVisible, AgCameraView::Handle view, std::vector<ScanContainerID>& voxelContainerListOut);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Adds a new scan to this project
		//////////////////////////////////////////////////////////////////////////
		bool				addScan(AgVoxelTreeRunTime* treePtr);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Unloads a project
		//////////////////////////////////////////////////////////////////////////
		bool                 unloadProject();

		//////////////////////////////////////////////////////////////////////////
		// \brief: if all scans have the same coordinate system string, return it, otherwise return an empty string
		//////////////////////////////////////////////////////////////////////////
		std::wstring getProjectOriginalCoordinateSystem() const;
		std::wstring getProjectCurrentCoordinateSystem() const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: set to global from world transform
		//////////////////////////////////////////////////////////////////////////
		void                                    setToGlobalFromWorldTransform(const RealityComputing::Common::RCTransform& toGlobalFromWorld);

		//////////////////////////////////////////////////////////////////////////
		// \brief: get to global from world transform
		//////////////////////////////////////////////////////////////////////////
		RealityComputing::Common::RCTransform        getToGlobalFromWorldTransform() const;


		//////////////////////////////////////////////////////////////////////////
		// \brief: Update bounding box of this project
		//////////////////////////////////////////////////////////////////////////
		void                            updateProjectBounds();


	protected:
		//////////////////////////////////////////////////////////////////////////
		// \brief: Perform frustum culling to determine the visible tree's,
		//  returns amount of visible scans
		//////////////////////////////////////////////////////////////////////////
		int			_DoPerformFrustumCulling(std::vector<PointCloudInformation> &visibleListOut);
		//////////////////////////////////////////////////////////////////////////
		// \brief: Returns the visible voxel containers from an individual scan
		//////////////////////////////////////////////////////////////////////////
		void        _GetVisibleNodesFromScan(int scan, std::vector<ScanContainerID>& voxelContainerListOut);
		//////////////////////////////////////////////////////////////////////////
		// \brief: Returns the visible voxel containers from the scan list
		//////////////////////////////////////////////////////////////////////////
		int         _GetVisibleNodesFromScans(const std::vector<PointCloudInformation> &visibleScanList,
			std::vector<ScanContainerID>& voxelContainerListOut);
		//////////////////////////////////////////////////////////////////////////
		// \brief: Reduce the amount of points that are streamed in, until it
		// falls below 'mMaxPointsLoad' returns the new number of
		// points that will be streamed in
		//////////////////////////////////////////////////////////////////////////
		int        _ReducePointCloudLoad(std::uint32_t amountOfPointsVisible, AgCameraView::Handle view, std::vector<ScanContainerID>& voxelContainerListOut);

	private:
		std::vector<AgVoxelTreeRunTime*>    m_scanList;

		POINTCLOUD_CULL_METHOD              mCullMethod;
		bool								mLightWeight;

		bool                                mCoordinateSystemHasChanged;

		//Maximum amount of points to be streamed in/ display at any given time, in millions
		std::uint32_t                       mMaxPointsLoad;

		double								m_geoReference[3];
		RealityComputing::Common::RCTransform         mToGlobalFromWorld;

		RealityComputing::Common::RCBox                   m_visibleProjectBounds;
		RealityComputing::Common::RCBox                   m_projectBounds;

		bool                                mIsProjectDirty;
	};
}
