#pragma once
#include "AgPointCloudProject.h"
#include "AgPointCloudLoadThread.h"
//#include "AgProjectInformation.h"
#include <common/RCCode.h>

#include <xstring>

namespace ambergris {

	class AgRealityComputingManager
	{
	public:
		struct PointCloudLoadOptions
		{
			bool useFileHeaderTransform;

			PointCloudLoadOptions() : useFileHeaderTransform(true) {}
		};
	public:

		AgRealityComputingManager() : m_totalPointCount(0) {}

		/*
		Loads a point cloud file, returns a pointer to the new svo-tree or NULL otherwise,
		returns rcOK if succesful, returns rcFileNotExist if there are files not found,
		returns rcLegacyFileFormat if there are legacy files
		*/
		AgVoxelTreeRunTime::Handle loadPointCloudFile(const std::wstring& filename, RealityComputing::Common::RCCode& returnCode, const PointCloudLoadOptions& opts = PointCloudLoadOptions());

		/*
		Unloads a point cloud file by id as file name may be same.
		*/
		bool unloadPointCloudFile(const std::wstring& fileId);

		/*
		Loads world from project info manager, returns rcOK if succesful, returns rcFileNotExist if there are files not found,
		returns rcLegacyFileFormat if there are legacy files
		*/
		RealityComputing::Common::RCCode loadProject(std::wstring fileName, bool autoSetGeoRef = false);

		/*
		Unloads a project
		*/
		bool unLoadProject();

		std::vector<std::wstring> getSearchPaths() const {	return mSearchPaths;	}

		////////////////////////////////////////////////////////////////////////
		//	Sets the global offset( for geo-referencing ) 
		////////////////////////////////////////////////////////////////////////
		void setGeoReference(const RealityComputing::Common::RCVector3d &geoReference);
		void autoSetGeoReference();
		bool hasGeoReference();

		void                        setLightWeight(bool val) { mLightWeight = val; }
		bool                        getLightWeight() const { return mLightWeight; }

		void                        setIgnoreClip(bool val) { mIgnoreClip = val; }
		bool                        getIgnoreClip() const { return mIgnoreClip; }

		const AgPointCloudProject& getPointCloudProject() const { return m_pointCloudProject; }
		AgPointCloudProject& getPointCloudProject() { return m_pointCloudProject; }
	protected:
		void _UpdateBounds();
		void _UpdateVisibleNodes(AgCameraView::Handle view);
		void _SetPointRequestsForVisibleNodes();
	protected:
		AgPointCloudProject m_pointCloudProject;

		std::wstring                m_projectFile;
		std::vector<std::wstring>               mSearchPaths;

		RealityComputing::Common::RCBox		m_worldBounds;
		RealityComputing::Common::RCBox     mTransformedWorldBounds;

		//std::vector<AgVoxelTreeNode>    mVoxelTreeNodes;
		std::vector<ScanContainerID> mVisibleNodes;
		std::unique_ptr<AgPointCloudRequests> mPointCloudRequests;

		std::uint32_t m_totalPointCount;

		bool                            mLightWeight;
		bool                            mIgnoreClip;
	};
}