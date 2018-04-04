#pragma once
#include "AgPointCloudProject.h"
#include <common/RCCode.h>

#include <xstring>

namespace ambergris {

	class AgProjectInfoMgr;

	class AgRealityComputingManager
	{
	public:
		struct PointCloudLoadOptions
		{
			bool useFileHeaderTransform;

			PointCloudLoadOptions() : useFileHeaderTransform(true) {}
		};
	public:

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
		RealityComputing::Common::RCCode loadProject(const AgProjectInfoMgr& infoMgr, bool autoSetGeoRef = false);

	protected:
		AgPointCloudProject m_pointCloudProject;

		std::wstring                m_projectFile;

		RealityComputing::Common::RCBox		m_worldBounds;
		RealityComputing::Common::RCBox     mTransformedWorldBounds;
	};
}