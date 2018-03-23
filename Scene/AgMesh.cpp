#include "AgMesh.h"
#include "Resource/AgRenderResourceManager.h"

namespace ambergris {

	void AgMesh::evaluateBoundingBox(AgBoundingbox* bbox)
	{
		if (!bbox)
			return;

		for (int i = 0; i < m_geometries.size(); ++i)
		{
			const AgGeometry* pGeometry = Singleton<AgRenderResourceManager>::instance().m_geometries.get(m_geometries.at(i));
			if (!pGeometry)
				continue;

			const AgVertexBuffer* vb = Singleton<AgRenderResourceManager>::instance().m_vertex_buffer_pool.get(pGeometry->vertex_buffer_handle);
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
				bbox->m_sphere = maxSphere;
			}
			else
			{
				bbox->m_sphere = minSphere;
			}

			toAabb(bbox->m_aabb, vb->m_vertex_buffer.GetData(), vertexNum, stride);
			calcObb(bbox->m_obb, vb->m_vertex_buffer.GetData(), vertexNum, stride);
		}
		bbox->m_prepared = true;
	}
}