#include "AgRenderMeshEvaluator.h"
#include "AgRenderInstanceNode.h"
#include "AgRenderSceneBridge.h"
#include "AgRenderer.h"

#include <memory>

namespace ambergris {

	/*virtual*/
	bool AgRenderMeshEvaluator::evaluate(const AgObject* pObject)
	{
		const AgMesh* mesh = dynamic_cast<const AgMesh*>(pObject);
		if (!mesh)
			return false;

		bool ret = true;
		bool instanceSupport = 0 != (bgfx::getCaps()->supported & BGFX_CAPS_INSTANCING);
		if (instanceSupport && mesh->m_inst_handle >= 0)
		{
			// Instance
			typedef std::vector<uint32_t> EvaNodeIdArr;
			EvaNodeIdArr evaNodes;
			for (int subMeshId = 0; subMeshId < mesh->m_geometries.size(); ++subMeshId)
			{
				for (uint32_t i = 0; i < m_evaluate_mapping.size(); ++i)
				{
					std::shared_ptr<ObjectEvaluatorInfo> evaluatorInfo = m_evaluate_mapping.at(i);
					if (!evaluatorInfo)
						continue;
					std::shared_ptr<MeshEvaluatorInfo> meshEvaluator =
						std::dynamic_pointer_cast<MeshEvaluatorInfo>(evaluatorInfo);
					if (!meshEvaluator)
						continue;
					if (meshEvaluator->m_geometry == mesh->m_geometries[subMeshId])
					{
						evaNodes.push_back(i);
						break;
					}
				}
			}

			static const int stride = 16;
			float instanceData[stride];
			ret &= (0 != memcpy_s(instanceData, stride * sizeof(float), mesh->m_global_transform, stride * sizeof(float)));
			instanceData[3] = mesh->m_pick_id[0] / 255.0f;
			instanceData[7] = mesh->m_pick_id[1] / 255.0f;
			instanceData[11] = mesh->m_pick_id[2] / 255.0f;

			if (evaNodes.empty())
			{
				uint32_t evaOffset = (uint32_t)m_evaluate_mapping.size();
				//Is first node
				ret &= _EvaluateSubMesh<AgRenderInstanceNode>(mesh, m_evaluate_mapping);
				if (evaOffset < m_evaluate_mapping.size())
				{
					for (uint32_t i = evaOffset; i < m_evaluate_mapping.size(); ++i)
					{
						std::shared_ptr<ObjectEvaluatorInfo> evaluatorInfo = m_evaluate_mapping.at(i);
						if (!evaluatorInfo)
							continue;
						std::shared_ptr<AgRenderInstanceNode> inst_node =
							std::dynamic_pointer_cast<AgRenderInstanceNode>(evaluatorInfo->m_pRenderNode);
						inst_node->appendInstance(instanceData, stride);
					}
				}
			}
			else
			{
				for (auto itm = evaNodes.cbegin(); itm != evaNodes.cend(); itm++)
				{
					std::shared_ptr<ObjectEvaluatorInfo> evaluatorInfo = m_evaluate_mapping.at(*itm);
					if (!evaluatorInfo)
						continue;

					std::shared_ptr<AgRenderInstanceNode> inst_node =
						std::dynamic_pointer_cast<AgRenderInstanceNode>(evaluatorInfo->m_pRenderNode);
					if (inst_node)
					{
						ret &= inst_node->appendInstance(instanceData, stride);
						evaluatorInfo->m_objectHandles.push_back(mesh->m_handle);
					}
				}
			}
		}
		else
		{
			ret &= _EvaluateSubMesh<AgRenderSingleNode>(mesh, m_evaluate_mapping);
		}
		return ret;
	}

	/*virtual*/ 
	void AgRenderMeshEvaluator::bridgeRenderer(AgRenderer& renderer) const
	{
		for (uint32_t i = 0; i < m_evaluate_mapping.size(); ++i)
		{
			std::shared_ptr<AgRenderEvaluator::ObjectEvaluatorInfo> evaluator = m_evaluate_mapping.at(i);
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