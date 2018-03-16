#include "AgRenderMeshBatchEvaluator.h"
#include "AgRenderSceneBridge.h"
#include "AgRenderer.h"

namespace ambergris {

	uint64_t  AgRenderMeshBatchEvaluator::_ComputeBatchHashCode(const AgMesh::Geometry* geom)
	{
		if (!geom)
			return 0;

		uint64_t hash_code = 2;
		for (uint8_t tex_stage = 0; tex_stage < AgShader::MAX_TEXTURE_SLOT_COUNT; tex_stage++)
		{
			AgTexture::Handle texture_handle = geom->texture_handle[tex_stage];
			if (AgTexture::kInvalidHandle == texture_handle)
				break;
			hash_code = hash_code * 31 + texture_handle;
		}

		return hash_code;
	}

	/*virtual*/
	bool AgRenderMeshBatchEvaluator::evaluate(const AgObject* pObject)
	{
		const AgMesh* mesh = dynamic_cast<const AgMesh*>(pObject);
		if (!mesh)
			return false;

		bool ret = true;
		bool instanceSupport = 0 != (bgfx::getCaps()->supported & BGFX_CAPS_INSTANCING);
		if (instanceSupport && mesh->m_inst_handle >= 0)
		{
			ret &= AgRenderMeshEvaluator::evaluate(pObject);
		}
		else
		{
			ret &= _BatchEvaluateSubMesh(mesh, m_batch_evaluate_mapping);
		}
		return ret;
	}

	/*virtual*/
	void AgRenderMeshBatchEvaluator::bridgeRenderer(AgRenderer& renderer) const
	{
		AgRenderMeshEvaluator::bridgeRenderer(renderer);

		for (auto iter = m_batch_evaluate_mapping.cbegin(); iter != m_batch_evaluate_mapping.cend(); ++iter)
		{
			std::shared_ptr<AgRenderEvaluator::ObjectEvaluatorInfo> evaluator = iter->second;
			if (!evaluator)
				continue;
			std::shared_ptr<AgRenderNode> pNode = evaluator->m_pRenderNode;
			if (pNode && pNode->prepare())
			{
				renderer.appendNode(pNode, evaluator->m_objectHandles);
			}
		}
	}
}