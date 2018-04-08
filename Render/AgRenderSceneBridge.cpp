#include "AgRenderMeshBatchEvaluator.h"
#include "AgRenderPointCloudEvaluator.h"
#include "Scene/AgSceneDatabase.h"

namespace ambergris {

	bool AgRenderSceneBridge()
	{
		AgRenderer& renderer = Singleton<AgRenderer>::instance();
		AgSceneDatabase& sceneDB = Singleton<AgSceneDatabase>::instance();

		renderer.m_isEvaluating = true;
		bool ret = true;

		AgRenderMeshEvaluator meshEvaluator;
		const int nNodeNum = (const int)sceneDB.m_objectManager.getSize();
		for (int i = 0; i < nNodeNum; i++)
		{
			AgObject* pObj = sceneDB.m_objectManager.get(i);
			if(!pObj || AgObject::kInvalidHandle == pObj->m_handle)
				continue;

			if (typeid(*pObj) == typeid(AgMesh))
			{
				ret &= meshEvaluator.evaluate(renderer, AgDrawInfo(pObj->m_handle, 0, 0, 0));
			}
		}

		// Point Cloud
		std::vector<AgDrawInfo> pointList;
		AgPointCloudProject& pointCloud = sceneDB.getPointCloudProject();
		pointCloud.evaluate(pointList, renderer.getActiveView(), sceneDB.getVisibleNodes());

		AgRenderPointCloudEvaluator pointCloudEvaluator;
		for (int i = 0; i < pointList.size(); i++)
		{
			AgVoxelTreeRunTime* pVoxelTree = pointCloud.get(pointList.at(i).mObject);
			if (!pVoxelTree || AgVoxelTreeRunTime::kInvalidHandle == pVoxelTree->m_handle)
				continue;

			ret &= pointCloudEvaluator.evaluate(renderer, pointList.at(i));
		}

		renderer.m_isEvaluating = false;
		sceneDB.m_objectManager.m_dirty = false;
		return ret;
	}
}