#include "AgRenderMeshEvaluator.h"
#include "AgRenderInstanceNode.h"
#include "AgRenderSceneBridge.h"
#include "AgRenderer.h"

#include <memory>

namespace ambergris {

	/*virtual*/
	bool AgRenderMeshEvaluator::evaluate(AgRenderer& renderer, const AgObject* pObject)
	{
		const AgMesh* mesh = dynamic_cast<const AgMesh*>(pObject);
		if (!mesh)
			return false;

		bool ret = true;
		bool instanceSupport = 0 != (bgfx::getCaps()->supported & BGFX_CAPS_INSTANCING);
		if (instanceSupport && mesh->m_inst_handle >= 0)
		{
			// Instance
			for (int subMeshId = 0; subMeshId < mesh->m_geometries.size(); ++subMeshId)
			{
				AgRenderNode* renderNode = const_cast<AgRenderNode*>(renderer.getRenderNode(mesh->m_geometries.at(subMeshId)));
				if (nullptr == renderNode)
				{
					//Is first node
					ret &= _EvaluateMeshImpl<AgRenderInstanceNode>(renderer, mesh);
				}
				else
				{
					AgRenderInstanceNode* inst_node =
						dynamic_cast<AgRenderInstanceNode*>(renderNode);
					if (inst_node)
					{
						static const int stride = 16;
						float instanceData[stride];
						ret &= (0 != memcpy_s(instanceData, stride * sizeof(float), mesh->m_global_transform, stride * sizeof(float)));
						instanceData[3] = mesh->m_pick_id[0] / 255.0f;
						instanceData[7] = mesh->m_pick_id[1] / 255.0f;
						instanceData[11] = mesh->m_pick_id[2] / 255.0f;

						ret &= inst_node->appendInstance(instanceData, stride);
					}
				}
			}
		}
		else
		{
			ret &= _EvaluateMeshImpl<AgRenderSingleNode>(renderer, mesh);
		}
		return ret;
	}

}