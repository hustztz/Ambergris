#include "BgfxRenderPipeline.h"

namespace ambergris_bgfx {

	void BgfxRenderPipeline::BgfxRenderQueue::Clear()
	{
		for each (auto node in m_renderNodes)
		{
			node.DestroyGeometry();
		}
		m_renderNodes.clear();
	}

	void BgfxRenderPipeline::BgfxRenderQueue::AppendNode(BgfxRenderNode* renderNode)
	{
		if(renderNode)
			m_renderNodes.push_back(renderNode);
	}


	BgfxRenderPipeline::BgfxRenderPipeline()
	{
	}

	BgfxRenderPipeline::~BgfxRenderPipeline()
	{
		Reset();
	}

	void BgfxRenderPipeline::Draw(bgfx::ViewId view) const
	{
		for (int i = 0; i < E_COUNT; i ++)
		{
			for (BgfxRenderQueue::RenderNodeArray::const_iterator node = m_queues[i].m_renderNodes.begin(), nodeEnd = m_queues[i].m_renderNodes.end(); node != nodeEnd; ++node)
			{
				node->Draw(view);
			}
		}
	}

	void BgfxRenderPipeline::Clear(int stage)
	{
		if (stage < 0 || stage >= E_COUNT)
			return;

		m_queues[stage].Clear();
	}

	void BgfxRenderPipeline::Reset()
	{
		for (int i = 0; i < E_COUNT; i++)
		{
			Clear(i);
		}
	}

	void BgfxRenderPipeline::AppendNode(Stage stage, BgfxRenderNode* renderNode)
	{
		if (stage <= 0 || stage > E_COUNT)
			stage = E_SCENE_OPAQUE;
		m_queues[stage].AppendNode(renderNode);
	}
}