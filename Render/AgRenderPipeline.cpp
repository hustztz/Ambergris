#include "AgRenderPipeline.h"

namespace ambergris {

	void AgRenderPipeline::AgRenderQueue::Clear()
	{
		for each (auto node in m_renderNodes)
		{
			if (!node)
				continue;
			node->DestroyGeometry();
			node.reset();
		}
		m_renderNodes.clear();
	}

	void AgRenderPipeline::AgRenderQueue::AppendNode(std::shared_ptr<AgRenderNode> renderNode)
	{
		if(renderNode)
			m_renderNodes.push_back(renderNode);
	}


	AgRenderPipeline::AgRenderPipeline()
	{
	}

	AgRenderPipeline::~AgRenderPipeline()
	{
		Reset();
	}

	void AgRenderPipeline::Draw(bgfx::ViewId view) const
	{
		for (int i = 0; i < E_COUNT; i ++)
		{
			for (AgRenderQueue::RenderNodeArray::const_iterator node = m_queues[i].m_renderNodes.begin(), nodeEnd = m_queues[i].m_renderNodes.end(); node != nodeEnd; ++node)
			{
				const AgRenderNode* itm = node->get();
				if(itm)
					itm->Draw(view);
			}
		}
	}

	void AgRenderPipeline::Clear(int stage)
	{
		if (stage < 0 || stage >= E_COUNT)
			return;

		m_queues[stage].Clear();
	}

	void AgRenderPipeline::Reset()
	{
		for (int i = 0; i < E_COUNT; i++)
		{
			Clear(i);
		}
	}

	void AgRenderPipeline::AppendNode(Stage stage, std::shared_ptr<AgRenderNode> renderNode)
	{
		if (stage <= 0 || stage > E_COUNT)
			stage = E_SCENE_OPAQUE;
		m_queues[stage].AppendNode(renderNode);
	}
}