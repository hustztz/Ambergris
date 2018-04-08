#include "AgRenderMeshEvaluator.h"
#include "AgRenderInstanceNode.h"
#include "AgRenderSceneBridge.h"
#include "AgRenderer.h"
#include "Scene/AgSceneDatabase.h"

#include <memory>

namespace ambergris {

	/*virtual*/
	bool AgRenderMeshEvaluator::evaluate(AgRenderer& renderer, AgDrawInfo drawInfo)
	{
		AgObject* pObject = Singleton<AgSceneDatabase>::instance().m_objectManager.get(drawInfo.mObject);
		if (!pObject || AgObject::kInvalidHandle == pObject->m_handle || !pObject->m_dirty)
			return false;

		const AgMesh* mesh = dynamic_cast<const AgMesh*>(pObject);
		if (!mesh)
			return false;

		pObject->m_dirty = false;

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
						const AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(mesh->m_global_transform_h);
						if (transform)
						{
							static const int stride = 16;
							float instanceData[stride];
							transform->getFloatTransform(instanceData);
							instanceData[3] = mesh->m_pick_id[0] / 255.0f;
							instanceData[7] = mesh->m_pick_id[1] / 255.0f;
							instanceData[11] = mesh->m_pick_id[2] / 255.0f;

							ret &= inst_node->appendInstance(instanceData, stride);
						}
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