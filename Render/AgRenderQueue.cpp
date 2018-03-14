#include "AgRenderQueue.h"

namespace ambergris {

	/*virtual*/
	void AgRenderQueue::destroy()
	{
		for (int i = 0; i < getSize(); ++i)
		{
			AgRenderNode* node = get(i);
			if (!node)
				continue;
			node->destroy();
		}
		AgResourcePool<AgRenderNode>::destroy();
	}

	void AgRenderQueueManager::destroy()
	{
		for (int i = 0; i < E_TYPE_COUNT; i++)
		{
			m_queues[i].destroy();
		}
	}

	AgRenderNode::Handle AgRenderQueueManager::appendNode(std::shared_ptr<AgRenderNode> renderNode)
	{
		if (!renderNode)
			return AgRenderNode::kInvalidHandle;

		switch (renderNode->getMaterial())
		{
		case AgMaterial::E_LAMBERT:
		case AgMaterial::E_PHONG:
			return m_queues[E_STATIC_SCENE_OPAQUE].append(renderNode);
		case 3:
			return m_queues[E_STATIC_SCENE_TRANSPARENT].append(renderNode);
		default:
			return m_queues[E_STATIC_SCENE_OPAQUE].append(renderNode);
		}

		return AgRenderNode::kInvalidHandle;
	}
}