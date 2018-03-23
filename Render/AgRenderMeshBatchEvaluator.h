#pragma once
#include "AgRenderMeshEvaluator.h"
#include "AgRenderMultiNode.h"

#include <map>

namespace ambergris {

	class AgRenderMeshBatchEvaluator : public AgRenderMeshEvaluator
	{
	public:
		AgRenderMeshBatchEvaluator()
			: AgRenderMeshEvaluator()	{}
		~AgRenderMeshBatchEvaluator() {}

		virtual bool evaluate(AgRenderer& renderer, const AgObject* pObject) override;

	protected:
#ifdef USING_TINYSTL
		typedef  stl::map<uint64_t, AgGeometry::Handle> BatchInfoMap;
#else
		typedef  std::map<uint64_t, AgGeometry::Handle> BatchInfoMap;
#endif
		BatchInfoMap			m_batch_mapping;

	protected:
		static uint64_t _ComputeBatchHashCode(const AgGeometry* geom);

		bool _AppendBatchRenderNode(AgRenderer& renderer, AgGeometry::Handle geom, const AgMesh* mesh)
		{
			if (!mesh)
				return false;

			const AgGeometry* pGeometry = Singleton<AgRenderResourceManager>::instance().m_geometries.get(geom);
			if (!pGeometry)
				return false;

			const AgVertexBuffer* vb = Singleton<AgRenderResourceManager>::instance().m_vertex_buffer_pool.get(pGeometry->vertex_buffer_handle);
			const AgIndexBuffer* ib = Singleton<AgRenderResourceManager>::instance().m_index_buffer_pool.get(pGeometry->index_buffer_handle);
			if (!vb || !ib)
				return false;

			const uint64_t hash_code = _ComputeBatchHashCode(pGeometry);
			const auto batchInfoIter = m_batch_mapping.find(hash_code);
			if (batchInfoIter == m_batch_mapping.cend())
			{
				std::shared_ptr<AgRenderMultiNode> renderNode(BX_NEW(entry::getAllocator(), AgRenderMultiNode));
				for (uint8_t tex_stage = 0; tex_stage < AgShader::MAX_TEXTURE_SLOT_COUNT; tex_stage++)
				{
					AgTexture::Handle texture_handle = pGeometry->texture_handle[tex_stage];
					if (AgTexture::kInvalidHandle == texture_handle)
						break;
					renderNode->setTexture(tex_stage, texture_handle);
				}
				if (renderNode->appendGeometry(
					mesh->m_global_transform,
					pGeometry->material_handle,
					mesh->m_bbox,
					mesh->m_pick_id,
					vb->m_decl,
					vb->m_vertex_buffer.GetData(), vb->m_vertex_buffer.GetSize() * sizeof(uint8_t),
					ib->m_index_buffer.GetData(), ib->m_index_buffer.GetSize() * sizeof(uint16_t)))
				{
					if (renderer.appendNode(renderNode, geom))
					{
						return true;
					}
				}
			}
			else
			{
				AgRenderNode* renderNode = const_cast<AgRenderNode*>(renderer.getRenderNode(batchInfoIter->second));
				if (renderNode)
				{
					if (renderNode->appendGeometry(
						mesh->m_global_transform,
						pGeometry->material_handle,
						mesh->m_bbox,
						mesh->m_pick_id,
						vb->m_decl,
						vb->m_vertex_buffer.GetData(), vb->m_vertex_buffer.GetSize() * sizeof(uint8_t),
						ib->m_index_buffer.GetData(), ib->m_index_buffer.GetSize() * sizeof(uint16_t)))
					{
						return true;
					}
				}
			}
			
			return false;
		}

		bool _BatchEvaluateMeshImpl(AgRenderer& renderer, const AgMesh* mesh)
		{
			if (!mesh)
				return false;

			const int nSubMeshNum = (const int)mesh->m_geometries.size();
			if (0 == nSubMeshNum)
				return false;

			for (int i = 0; i < nSubMeshNum; i++)
			{
				_AppendBatchRenderNode(renderer, mesh->m_geometries.at(i), mesh);
			}
			return true;
		}
	};
}
