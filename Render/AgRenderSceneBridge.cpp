#include "AgRenderSceneBridge.h"
#include "AgRenderMeshEvaluator.h"
#include "AgRenderInstanceNode.h"
#include "AgRenderPointCloudEvaluator.h"
#include "AgRenderer.h"
#include "Scene/AgSceneDatabase.h"

namespace ambergris {

	bool AgRenderSceneBridge(AgRenderer& renderer, const AgSceneDatabase& sceneDB)
	{
		bool ret = true;
		AgRenderBaseEvaluator::EvaluateNodeArr eva_node_arr;
		const int nNodeNum = (const int)sceneDB.getSize();
		for (int i = 0; i < nNodeNum; i++)
		{
			const AgMesh* mesh = dynamic_cast<const AgMesh*>(sceneDB.get(i));
			if (!mesh)
				continue;
			if (mesh->m_inst_handle >= 0)
			{
				bool ret = true;
				// Instance
				for (int subMeshId = 0; subMeshId < mesh->m_geometries.size(); ++subMeshId)
				{

					bool bIsFirstNode = true;
					for (int i = 0; i < eva_node_arr.size(); ++i)
					{
						if (eva_node_arr[i].m_geometry == mesh->m_geometries[subMeshId])
							bIsFirstNode = false;
					}
					if (bIsFirstNode)
					{
						std::unique_ptr<AgRenderBaseEvaluator> evaluator(new AgRenderMeshEvaluator<AgRenderInstanceNode>(eva_node_arr));
						ret &= evaluator->evaluate(*mesh);
					}
					else
					{
						float instanceData[20];
						int stride = 20 * sizeof(float);
						ret &= (0 != memcpy_s(instanceData, stride, mesh->m_global_transform, 16 * sizeof(float)));
						for (int i = 0; i < eva_node_arr.size(); ++i)
						{
							if (eva_node_arr[i].m_geometry == mesh->m_geometries[subMeshId])
							{
								std::shared_ptr<AgRenderInstanceNode> inst_node =
									std::dynamic_pointer_cast<AgRenderInstanceNode>(eva_node_arr[i].m_renderNode);
								if (inst_node)
									ret &= inst_node->appendInstance(instanceData, stride);
							}
						}
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
			std::shared_ptr<AgRenderNode> renderNode = eva_node_arr[i].m_renderNode;
			if(renderNode && renderNode->prepare())
				renderer.appendNode(renderNode);
		}
		return ret;
	}
}