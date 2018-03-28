#include "AgRenderMeshBatchEvaluator.h"
#include "Scene/AgSceneDatabase.h"

namespace ambergris {

	bool AgRenderSceneBridgeImpl(AgRenderer& renderer, AgSceneDatabase& sceneDB)
	{
		renderer.m_isEvaluating = true;
		bool ret = true;

		AgRenderMeshEvaluator meshEvaluator;
		const int nNodeNum = (const int)sceneDB.m_objectManager.getSize();
		for (int i = 0; i < nNodeNum; i++)
		{
			AgObject* pObj = sceneDB.m_objectManager.get(i);
			if(!pObj || AgObject::kInvalidHandle == pObj->m_handle || !pObj->m_dirty)
				continue;

			pObj->m_dirty = false;

			if (typeid(*pObj) == typeid(AgMesh))
			{
				meshEvaluator.evaluate(renderer, pObj);
			}
		}

		renderer.m_isEvaluating = false;
		sceneDB.m_objectManager.m_dirty = false;
		return ret;
	}

	bool AgRenderSceneBridge()
	{
		AgRenderer& renderer = Singleton<AgRenderer>::instance();
		AgSceneDatabase& sceneDB = Singleton<AgSceneDatabase>::instance();
		return AgRenderSceneBridgeImpl(renderer, sceneDB);
	}
}