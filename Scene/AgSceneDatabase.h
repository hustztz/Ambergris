#pragma once
#include "AgMesh.h"
#include "PointCloud/AgPointCloudProject.h"
#include "Foundation/Singleton.h"
#include "Foundation/AgWorldCoordInfo.h"

#ifdef USING_TINYSTL
#include <tinystl/unordered_map.h>
#else
#include <unordered_map>
#endif


namespace ambergris {

	struct AgSceneDatabase
	{
	public:
		AgObject::Handle appendObject(std::shared_ptr<AgObject> object);
	private:
		AgSceneDatabase() {}
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

		AgObjectManager			m_objectManager;
		AgPointCloudProject		m_pointCloud;

		AgWorldCoordInfo mWorldCoordInfo;
	};
}