#pragma once
#include "AgRenderMeshEvaluator.h"
#include "AgRenderBatchNode.h"

#include <map>

namespace ambergris {

	class AgRenderMeshBatchEvaluator : public AgRenderMeshEvaluator
	{
	public:
		AgRenderMeshBatchEvaluator()
			: AgRenderMeshEvaluator()	{}
		~AgRenderMeshBatchEvaluator() {}

		virtual bool evaluate(const AgObject* pObject) override;
		virtual void bridgeRenderer(AgRenderer& renderer) const override;

	protected:
#ifdef USING_TINYSTL
		typedef  stl::map<uint64_t, std::shared_ptr<ObjectEvaluatorInfo>> EvaluateInfoMap;
#else
		typedef  std::map<uint64_t, std::shared_ptr<ObjectEvaluatorInfo>> EvaluateInfoMap;
#endif
		EvaluateInfoMap			m_batch_evaluate_mapping;

	protected:
		static uint64_t _ComputeBatchHashCode(const AgMesh::Geometry* geom);

		bool _AppendBatchRenderNode(int subId, const AgMesh* mesh, EvaluateInfoMap& evaluatorInfos)
		{
			if (!mesh)
				return false;

			const AgVertexBuffer* vb = Singleton<AgRenderResourceManager>::instance().m_vertex_buffer_pool.get(mesh->m_geometries[subId].vertex_buffer_handle);
			const AgIndexBuffer* ib = Singleton<AgRenderResourceManager>::instance().m_index_buffer_pool.get(mesh->m_geometries[subId].index_buffer_handle);
			if (!vb || !ib)
				return false;

			const uint64_t hash_code = _ComputeBatchHashCode(&mesh->m_geometries[subId]);
			const auto evaluatorInfoIter = m_batch_evaluate_mapping.find(hash_code);
			if (evaluatorInfoIter == m_batch_evaluate_mapping.cend())
			{
				std::shared_ptr<AgRenderBatchNode> renderNode(BX_NEW(entry::getAllocator(), AgRenderBatchNode));
				for (uint8_t tex_stage = 0; tex_stage < AgShader::MAX_TEXTURE_SLOT_COUNT; tex_stage++)
				{
					AgTexture::Handle texture_handle = mesh->m_geometries[subId].texture_handle[tex_stage];
					if (AgTexture::kInvalidHandle == texture_handle)
						break;
					renderNode->setTexture(tex_stage, texture_handle);
				}
				if (!renderNode->appendGeometry(
					mesh->m_global_transform,
					mesh->m_geometries[subId].material_handle,
					mesh->m_pick_id,
					vb->m_decl,
					vb->m_vertex_buffer.GetData(), vb->m_vertex_buffer.GetSize() * sizeof(uint8_t),
					ib->m_index_buffer.GetData(), ib->m_index_buffer.GetSize() * sizeof(uint16_t)))
				{
					return false;
				}
				std::shared_ptr<AgRenderMeshEvaluator::ObjectEvaluatorInfo> evaluatorInfo(new AgRenderMeshEvaluator::ObjectEvaluatorInfo(renderNode, mesh->m_handle));
				evaluatorInfos.insert(std::pair< uint64_t, std::shared_ptr<ObjectEvaluatorInfo> >(hash_code, evaluatorInfo));
			}
			else
			{
				std::shared_ptr<ObjectEvaluatorInfo> evaluatorInfo = evaluatorInfoIter->second;
				if (evaluatorInfo)
				{
					std::shared_ptr<AgRenderNode> renderNode = evaluatorInfo->m_pRenderNode;
					if (renderNode)
					{
						if (!renderNode->appendGeometry(
							mesh->m_global_transform,
							mesh->m_geometries[subId].material_handle,
							mesh->m_pick_id,
							vb->m_decl,
							vb->m_vertex_buffer.GetData(), vb->m_vertex_buffer.GetSize() * sizeof(uint8_t),
							ib->m_index_buffer.GetData(), ib->m_index_buffer.GetSize() * sizeof(uint16_t)))
						{
							return false;
						}
					}
				}
			}
			
			return true;
		}

		bool _BatchEvaluateSubMesh(const AgMesh* mesh, EvaluateInfoMap& evaluatorInfos)
		{
			if (!mesh)
				return false;

			const int nSubMeshNum = (const int)mesh->m_geometries.size();
			if (0 == nSubMeshNum)
				return false;

			for (int i = 0; i < nSubMeshNum; i++)
			{
				_AppendBatchRenderNode(i, mesh, evaluatorInfos);
			}
			return true;
		}
	};
}
