#pragma once

#include "Foundation/Singleton.h"
#include "AgMesh.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace ambergris {

	class AgSceneGeometryResource
	{
		struct GeometryInst
		{
			int				m_mesh;
			typedef std::vector<int> INSTANCES;
			INSTANCES		m_instances;
		};
	public:
		// Mesh
		void ClearMeshes() { m_meshes.clear(); }
		int AppendMesh(AgMesh* node) {
			m_meshes.push_back(node);
			return (int)GetMeshSize() - 1;
		}
		size_t GetMeshSize() const { return m_meshes.size(); }
		const AgMesh* GetMesh(int id) const {
			if(id >= 0 && id < GetMeshSize())
				return &m_meshes.at(id);
			return nullptr;
		}
		// Instance
		void ClearInstance() { m_inst_pool.clear(); }
		int AppendInstance(int mesh, int node);
		size_t GetInstanceSize(int id) const;
		int GetInstance(int id, int inst) const;
	private:
		AgSceneGeometryResource() {};
		~AgSceneGeometryResource() {
			ClearMeshes();
			ClearInstance();
		}
		friend class Singleton<AgSceneGeometryResource>;
	private:
		boost::ptr_vector<AgMesh>		m_meshes;
		boost::ptr_vector<GeometryInst>	m_inst_pool;
	};
}