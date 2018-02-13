#pragma once

#include "Foundation/Singleton.h"
#include "BgfxMesh.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace ambergris {

	class SceneGeometryResource
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
		int AppendMesh(BgfxMesh* node) {
			m_meshes.push_back(node);
			return (int)GetMeshSize() - 1;
		}
		size_t GetMeshSize() const { return m_meshes.size(); }
		const BgfxMesh* GetMesh(int id) const {
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
		SceneGeometryResource() {};
		~SceneGeometryResource() {
			ClearMeshes();
			ClearInstance();
		}
		friend class Singleton<SceneGeometryResource>;
	private:
		boost::ptr_vector<BgfxMesh>		m_meshes;
		boost::ptr_vector<GeometryInst>	m_inst_pool;
	};
}