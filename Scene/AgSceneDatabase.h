#pragma once
#include "Foundation/Singleton.h"
#include "Foundation/AgWorldCoordInfo.h"
#include "AgMesh.h"
#include "PointCloud/AgPointCloudProject.h"
#include "PointCloud/AgPointCloudLoadThread.h"
//#include "PointCloud/AgProjectInformation.h"

#include <common/RCCode.h>
#include <xstring>

#ifdef USING_TINYSTL
#include <tinystl/unordered_map.h>
#else
#include <unordered_map>
#endif


namespace ambergris {

	struct AgSceneDatabase
	{
	public:
		struct PointCloudLoadOptions
		{
			bool useFileHeaderTransform;

			PointCloudLoadOptions() : useFileHeaderTransform(true) {}
		};
	public:
		AgObject::Handle appendObject(std::shared_ptr<AgObject> object);

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

		std::vector<std::wstring> getSearchPaths() const { return mSearchPaths; }

		////////////////////////////////////////////////////////////////////////
		//	Sets the global offset( for geo-referencing ) 
		////////////////////////////////////////////////////////////////////////
		void setGeoReference(const RealityComputing::Common::RCVector3d &geoReference);
		void autoSetGeoReference();
		bool hasGeoReference();

		const AgPointCloudProject& getPointCloudProject() const { return m_pointCloudProject; }
		AgPointCloudProject& getPointCloudProject() { return m_pointCloudProject; }

		const std::vector<ScanContainerID>& getVisibleNodes() const { return mVisibleNodes; }
	protected:
		void _UpdateBounds();
		void _UpdateVisibleNodes(AgCameraView::Handle view);
		void _SetPointRequestsForVisibleNodes();
	private:
		AgSceneDatabase() : m_totalPointCount(0) {}
		~AgSceneDatabase() {}
		friend class Singleton<AgSceneDatabase>;
	public:
#ifdef USING_TINYSTL
		typedef stl::unordered_map<uint32_t, AgMesh::Handle> SelectMap;
#else
		typedef std::unordered_map<uint32_t, AgMesh::Handle> SelectMap;
#endif
		SelectMap		m_select_id_map;
#ifdef USING_TINYSTL
		typedef stl::vector<AgMesh::Handle> SelectResult;
#else
		typedef std::vector<AgMesh::Handle> SelectResult;
#endif
		SelectResult	m_select_result;

		AgObjectManager		m_objectManager;

		AgWorldCoordInfo mWorldCoordInfo;

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
	};
}