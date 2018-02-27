#include "AgRenderPipeline.h"

namespace ambergris {

	void AgRenderPipeline::AgRenderQueue::clear()
	{
		for each (auto node in m_renderNodes)
		{
			if (!node)
				continue;
			node->destroy();
			node.reset();
		}
		m_renderNodes.clear();
	}

	void AgRenderPipeline::AgRenderQueue::appendNode(std::shared_ptr<AgRenderNode> renderNode)
	{
		if(renderNode)
			m_renderNodes.push_back(renderNode);
	}


	AgRenderPipeline::AgRenderPipeline()
	{
	}

	AgRenderPipeline::~AgRenderPipeline()
	{
		reset();
	}

	void AgRenderPipeline::draw(bgfx::ViewId view) const
	{
		// TODO: multithread
		for (int i = 0; i < E_COUNT; i ++)
		{
			for (AgRenderQueue::RenderNodeArray::const_iterator node = m_queues[i].m_renderNodes.begin(), nodeEnd = m_queues[i].m_renderNodes.end(); node != nodeEnd; ++node)
			{
				const AgRenderNode* itm = node->get();
				if(itm)
					itm->draw(view);
			}
		}
	}

	void AgRenderPipeline::clear(int stage)
	{
		if (stage < 0 || stage >= E_COUNT)
			return;

		m_queues[stage].clear();
	}

	void AgRenderPipeline::reset()
	{
		for (int i = 0; i < E_COUNT; i++)
		{
			clear(i);
		}
	}

	void AgRenderPipeline::appendNode(Stage stage, std::shared_ptr<AgRenderNode> renderNode)
	{
		if (stage <= 0 || stage > E_COUNT)
			stage = E_STATIC_SCENE_OPAQUE;
		m_queues[stage].appendNode(renderNode);
	}
}