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

}