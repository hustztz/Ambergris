#include "AgRenderPointCloudEvaluator.h"
#include "AgRenderer.h"
#include "Resource\AgRenderResourceManager.h"
#include "Scene/AgSceneDatabase.h"
#include "Scene/PointCloud/AgVoxelTreeRunTime.h"

#include "AgRenderSingleNode.h"
#include "BGFX/entry/entry.h" //TODO

namespace ambergris {

	/*virtual*/
	bool AgRenderPointCloudEvaluator::evaluate(AgRenderer& renderer, AgDrawInfo drawInfo)
	{
		AgVoxelTreeRunTime* curTreePtr = Singleton<AgSceneDatabase>::instance().getPointCloudProject().get(drawInfo.mObject);
		if (!curTreePtr || AgVoxelTreeRunTime::kInvalidHandle == curTreePtr->m_handle)
			return false;

		AgVoxelContainer* pContainer = curTreePtr->getVoxelContainerAt(drawInfo.mContainerId);
		if (!pContainer)
			return false;

		const AgGeometry::Handle geom = pContainer->getGeometry();

		const AgGeometry* pGeometry = Singleton<AgRenderResourceManager>::instance().m_geometries.get(geom);
		if (!pGeometry || AgGeometry::kInvalidHandle == pGeometry->m_handle)
			return false;

		const AgVertexBuffer* vb = Singleton<AgRenderResourceManager>::instance().m_vertex_buffer_pool.get(pGeometry->vertex_buffer_handle);
		const AgIndexBuffer* ib = Singleton<AgRenderResourceManager>::instance().m_index_buffer_pool.get(pGeometry->index_buffer_handle);
		if (!vb || AgVertexBuffer::kInvalidHandle == vb->m_handle || !ib || AgIndexBuffer::kInvalidHandle == ib->m_handle)
			return false;

		std::shared_ptr<AgRenderSingleNode> renderNode(BX_NEW(entry::getAllocator(), AgRenderSingleNode));
		for (uint8_t tex_stage = 0; tex_stage < AgShader::MAX_TEXTURE_SLOT_COUNT; tex_stage++)
		{
			AgTexture::Handle texture_handle = pGeometry->texture_handle[tex_stage];
			if (AgTexture::kInvalidHandle == texture_handle)
				break;
			renderNode->setTexture(tex_stage, texture_handle);
		}
		if (!renderNode->appendGeometry(
			curTreePtr->m_global_transform_h,
			pGeometry->material_handle,
			curTreePtr->m_bbox,
			nullptr/*pickId*/,
			vb->m_decl,
			vb->m_vertex_buffer.GetData(), vb->m_vertex_buffer.GetSize() * sizeof(uint8_t),
			ib->m_index_buffer.GetData(), ib->m_index_buffer.GetSize() * sizeof(uint16_t)))
		{
			return false;
		}
		return nullptr != renderer.appendNode(renderNode, geom);

		return true;
	}
}