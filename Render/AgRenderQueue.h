#pragma once
#include "AgRenderNode.h"
#include "Resource/AgResourcePool.h"

namespace ambergris {
	class AgRenderQueue : public AgResourcePool<AgRenderNode>
	{
	public:
		virtual void destroy() override;
	};

	struct AgRenderQueueManager
	{
		enum QueueType
		{
			E_STATIC_SCENE_OPAQUE = 0,
			E_STATIC_SCENE_TRANSPARENT,
			E_WIREFRAME,

			E_TYPE_COUNT
		}; 
		static bool isScene(QueueType type) {
			return (E_STATIC_SCENE_OPAQUE == type) ||
				(E_STATIC_SCENE_TRANSPARENT == type)
				;
		}

		void destroy();
		AgRenderNode::Handle appendNode(std::shared_ptr<AgRenderNode> renderNode);
		
		AgRenderQueue m_queues[E_TYPE_COUNT];
	};
}