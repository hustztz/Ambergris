#include "AgMesh.h"
#include "Resource/AgRenderResourceManager.h"

#include "BGFX/entry/entry.h"//TODO

namespace ambergris {

	void AgMesh::_EvaluateBoundingBox(AgBoundingbox* bbox)
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

			/*if (minSphere.m_radius > maxSphere.m_radius)
			{
				bbox->m_sphere = maxSphere;
			}
			else
			{
				bbox->m_sphere = minSphere;
			}

			toAabb(bbox->m_aabb, vb->m_vertex_buffer.GetData(), vertexNum, stride);
			calcObb(bbox->m_obb, vb->m_vertex_buffer.GetData(), vertexNum, stride);*/
		}
	}

	void AgMesh::evaluateBoundingBox()
	{
		AgBoundingbox* oldbbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(m_bbox);
		if (!oldbbox)
		{
			std::shared_ptr<AgBoundingbox> bbox(BX_NEW(entry::getAllocator(), AgBoundingbox));
			_EvaluateBoundingBox(bbox.get());
			m_bbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.append(bbox);
		}
		else
		{
			_EvaluateBoundingBox(oldbbox);
		}
	}
}