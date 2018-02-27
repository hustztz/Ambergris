#include "AgMesh.h"
#include "Resource/AgGeometryResourceManager.h"

namespace ambergris {

	void AgMesh::evaluateBoundingBox()
	{
		for (int i = 0; i < m_geometries.size(); ++i)
		{
			const AgVertexBuffer* vb = Singleton<AgGeometryResourceManager>::instance().m_vertex_buffer_pool.get(m_geometries[i].vertex_buffer_handle);
			if (!vb)
				continue;

			const uint32_t stride = vb->m_decl.m_stride;
			const uint32_t vertexNum = (uint32_t)vb->m_vertex_buffer.GetSize() / (stride * sizeof(uint8_t));

			Sphere maxSphere;
			calcMaxBoundingSphere(maxSphere, vb->m_vertex_buffer.GetData(), vertexNum, stride);

			Sphere minSphere;
			calcMinBoundingSphere(minSphere, vb->m_vertex_buffer.GetData(), vertexNum, stride);

			if (minSphere.m_radius > maxSphere.m_radius)
			{
				m_bb.m_sphere = maxSphere;
			}
			else
			{
				m_bb.m_sphere = minSphere;
			}

			toAabb(m_bb.m_aabb, vb->m_vertex_buffer.GetData(), vertexNum, stride);
			calcObb(m_bb.m_obb, vb->m_vertex_buffer.GetData(), vertexNum, stride);
		}
	}
}