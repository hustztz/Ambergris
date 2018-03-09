#include "AgRenderSceneBridge.h"
#include "AgRenderMeshEvaluator.h"
#include "AgRenderInstanceNode.h"
#include "AgRenderPointCloudEvaluator.h"
#include "AgRenderer.h"
#include "Scene/AgSceneDatabase.h"

#include <memory>
#include <vector>

namespace ambergris {

	bool AgRenderSceneBridgeImpl(AgRenderer& renderer, AgSceneDatabase& sceneDB)
	{
		renderer.m_isEvaluating = true;
		bool ret = true;
		const bgfx::Caps* caps = bgfx::getCaps();
		bool instanceSupport = 0 != (caps->supported & BGFX_CAPS_INSTANCING);
		AgRenderBaseEvaluator::EvaluateNodeArr eva_node_arr;
		const int nNodeNum = (const int)sceneDB.getSize();
		for (int i = 0; i < nNodeNum; i++)
		{
			AgObject* pObj = sceneDB.get(i);
			if(!pObj || !pObj->m_dirty)
				continue;

			pObj->m_dirty = false;

			const AgMesh* mesh = dynamic_cast<const AgMesh*>(pObj);
			if (!mesh)
				continue;
			if (instanceSupport && mesh->m_inst_handle >= 0)
			{
				bool ret = true;
				// Instance
				typedef std::vector<uint32_t> EvaNodeIdArr;
				EvaNodeIdArr evaNodes;
				for (int subMeshId = 0; subMeshId < mesh->m_geometries.size(); ++subMeshId)
				{
					for (int i = 0; i < eva_node_arr.size(); ++i)
					{
						if (eva_node_arr[i].m_geometry == mesh->m_geometries[subMeshId])
						{
							evaNodes.push_back(i);
							break;
						}
					}
				}

				static const int stride = 16;
				float instanceData[stride];
				ret &= (0 != memcpy_s(instanceData, stride * sizeof(float), mesh->m_global_transform, stride * sizeof(float)));

				if (evaNodes.empty())
				{
					uint32_t evaOffset = (uint32_t)eva_node_arr.size();
					//Is first node
					std::unique_ptr<AgRenderBaseEvaluator> evaluator(new AgRenderMeshEvaluator<AgRenderInstanceNode>(eva_node_arr));
					ret &= evaluator->evaluate(*mesh);
					if (evaOffset < eva_node_arr.size())
					{
						for (uint32_t i = evaOffset; i < eva_node_arr.size(); ++i)
						{
							AgRenderInstanceNode* inst_node =
								dynamic_cast<AgRenderInstanceNode*>(eva_node_arr[i].m_pRenderNode);
							inst_node->appendInstance(instanceData, stride);
						}
					}
				}
				else
				{
					for (auto itm = evaNodes.cbegin(); itm != evaNodes.cend(); itm ++)
					{
						AgRenderInstanceNode* inst_node =
							dynamic_cast<AgRenderInstanceNode*>(eva_node_arr[*itm].m_pRenderNode);
						if (inst_node)
							ret &= inst_node->appendInstance(instanceData, stride);
					}
				}
			}
			else
			{
				std::unique_ptr<AgRenderBaseEvaluator> evaluator(new AgRenderMeshEvaluator<AgRenderNode>(eva_node_arr));
				ret &= evaluator->evaluate(*mesh);
			}
		}

		for (int i = 0; i < eva_node_arr.size(); ++i)
		{
			AgRenderNode* pNode = eva_node_arr[i].m_pRenderNode;
			if (pNode && pNode->prepare())
			{
				renderer.m_pipeline.appendNode(pNode);
			}
			else if (pNode)
			{
				delete pNode;
			}
		}
		renderer.m_isEvaluating = false;
		sceneDB.m_dirty = false;
		return ret;
	}

	bool AgRenderSceneBridge()
	{
		AgRenderer& renderer = Singleton<AgRenderer>::instance();
		AgSceneDatabase& sceneDB = Singleton<AgSceneDatabase>::instance();
		return AgRenderSceneBridgeImpl(renderer, sceneDB);
	}
}