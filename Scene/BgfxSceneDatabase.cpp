#include "BgfxSceneDatabase.h"
#include "SceneGeometryResource.h"

namespace ambergris {

	void BgfxSceneDatabase::Reset()
	{
		m_node_arr.clear();
	}

	void BgfxSceneDatabase::AppendNode(BgfxNodeHeirarchy* node)
	{
		if (!node)
			return;

		const BgfxMesh* pMesh = Singleton<ambergris::SceneGeometryResource>::instance().GetMesh(node->m_mesh_handle);
		if (!pMesh)
			return;

		const uint32_t stride = pMesh->m_decl.m_stride;
		const uint32_t vertexNum = (uint32_t)pMesh->m_vertex_buffer.GetSize() / (stride * sizeof(uint8_t));

		Sphere maxSphere;
		calcMaxBoundingSphere(maxSphere, pMesh->m_vertex_buffer.GetData(), vertexNum, stride);

		Sphere minSphere;
		calcMinBoundingSphere(minSphere, pMesh->m_vertex_buffer.GetData(), vertexNum, stride);

		if (minSphere.m_radius > maxSphere.m_radius)
		{
			node->m_bb.m_sphere = maxSphere;
		}
		else
		{
			node->m_bb.m_sphere = minSphere;
		}

		toAabb(node->m_bb.m_aabb, pMesh->m_vertex_buffer.GetData(), vertexNum, stride);
		calcObb(node->m_bb.m_obb, pMesh->m_vertex_buffer.GetData(), vertexNum, stride);


		int nNodeNum = (int)m_node_arr.size();
		for (int i = 0; i < nNodeNum; ++i)
		{
			if (m_node_arr[i].m_mesh_handle == node->m_mesh_handle)
			{
				node->m_inst_handle = Singleton<ambergris::SceneGeometryResource>::instance().AppendInstance(node->m_mesh_handle, nNodeNum);
				m_node_arr[i].m_inst_handle = node->m_inst_handle;
				break;
			}
		}
		
		m_node_arr.push_back(node);
	}
}