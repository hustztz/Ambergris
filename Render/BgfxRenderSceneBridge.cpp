#include "BgfxRenderSceneBridge.h"
#include "BgfxRenderEvaluator.h"
#include "BgfxRenderInstanceNode.h"
#include "BgfxBatchRenderEvaluator.h"
#include "BgfxRenderer.h"
#include "Scene/BgfxSceneDatabase.h"

#include <vector>

using namespace ambergris;

namespace ambergris_bgfx {

	bool BgfxRenderSceneBridge(BgfxRenderer& renderer, const BgfxSceneDatabase& sceneDB)
	{
		bool ret = true;
		BgfxRenderBaseEvaluator::EvaluateNodeArr eva_node_arr;
		const int nNodeNum = (const int)sceneDB.GetNodeSize();
		for (int i = 0; i < nNodeNum; i++)
		{
			const ambergris::BgfxNodeHeirarchy& geomNode = sceneDB.GetNode(i);
			if (geomNode.m_inst_handle >= 0)
			{
				// Instance
				bool bIsFirstNode = true;
				for (int i = 0; i < eva_node_arr.size(); ++i)
				{
					if (eva_node_arr.at(i).m_mesh_handle == geomNode.m_mesh_handle)
						bIsFirstNode = false;
				}

				bool ret = true;
				if (bIsFirstNode)
				{
					std::unique_ptr<BgfxRenderBaseEvaluator> evaluator(new BgfxRenderEvaluator<BgfxRenderInstanceNode>(eva_node_arr));
					ret &= evaluator->Evaluate(geomNode);
				}
				else
				{
					float instanceData[20];
					int stride = 20 * sizeof(float);
					ret &= (0 != memcpy_s(instanceData, stride, geomNode.m_global_transform, 16 * sizeof(float)));
					for (int i = 0; i < eva_node_arr.size(); ++i)
					{
						if (eva_node_arr.at(i).m_mesh_handle == geomNode.m_mesh_handle)
						{
							BgfxRenderInstanceNode* inst_node = dynamic_cast<BgfxRenderInstanceNode*>(eva_node_arr.at(i).m_renderNode);
							if (inst_node)
								ret &= inst_node->AppendInstance(instanceData, stride);
						}
					}
				}
			}
			else
			{
				std::unique_ptr<BgfxRenderBaseEvaluator> evaluator(new BgfxRenderEvaluator<BgfxRenderNode>(eva_node_arr));
				ret &= evaluator->Evaluate(geomNode);
			}
		}

		for (int i = 0; i < eva_node_arr.size(); ++i)
		{
			BgfxRenderNode* renderNode = eva_node_arr.at(i).m_renderNode;
			if(renderNode && renderNode->Prepare())
				renderer.AppendNode(renderNode);
		}
		return ret;
	}
}