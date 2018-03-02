#include "AgRenderPipeline.h"
#include "AgHardwarePickingSystem.h"

namespace ambergris {

	AgRenderPipeline::AgRenderPipeline()
		: m_occusion_cull(false)
		, m_pPicking(nullptr)
		, m_pick_reading(0)
		, m_pick_drawed(false)
		, m_currFrame(UINT32_MAX)
	{
		for (uint8_t i = 0; i < AgRenderPass::E_PASS_COUNT; i ++)
		{
			m_viewId[i] = i;
		}

		m_renderQueueManager.destroy();
	}

	AgRenderPipeline::~AgRenderPipeline()
	{
		if (m_pPicking)
		{
			delete m_pPicking;
			m_pPicking = nullptr;
		}
		m_renderQueueManager.destroy();
	}

	void AgRenderPipeline::reset()
	{
		for (uint8_t i = 0; i < AgRenderPass::E_PASS_COUNT; i++)
		{
			switch (i)
			{
			case AgRenderPass::E_PASS_ID:
				// ID buffer clears to black, which represnts clicking on nothing (background)
				bgfx::setViewClear(AgRenderPass::E_PASS_ID
					, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
					, 0x000000ff
					, 1.0f
					, 0
				);
				break;
			default:
				bgfx::setViewClear(i
					, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
					, 0x303030ff
					, 1.0f
					, 0
				);
				break;
			}
		}
		m_renderQueueManager.destroy();
	}

	AgRenderNode::Handle AgRenderPipeline::appendNode(AgRenderNode* renderNode)
	{
		return m_renderQueueManager.appendNode(renderNode);
	}

	void AgRenderPipeline::updatePickingInfo(float* invViewProj, float mouseXNDC, float mouseYNDC)
	{
		if(m_pPicking)
			m_pPicking->update(AgRenderPass::E_PASS_ID, invViewProj, mouseXNDC, mouseYNDC);
	}

	void AgRenderPipeline::run()
	{
		// Set view 0 default viewport.
		bgfx::setViewRect(0, 0, 0, bgfx::BackbufferRatio::Equal);

		// This dummy draw call is here to make sure that view 0 is cleared
		// if no other draw calls are submitted to view 0.
		for (uint8_t i = 0; i < AgRenderPass::E_PASS_COUNT; i++)
		{
			bgfx::touch(i);
		}

		// TODO: multithread
		for (int i = 0; i < AgRenderQueueManager::E_TYPE_COUNT; i ++)
		{
			for (int j = 0; j < m_renderQueueManager.m_queues[i].getSize(); ++j)
			{
				AgRenderNode* node = m_renderQueueManager.m_queues[i].get(j);
				if (!node)
					continue;

				if (AgRenderQueueManager::isScene((AgRenderQueueManager::QueueType)i))
				{
					node->draw(m_viewId[AgRenderPass::E_PASS_SHADING], m_occusion_cull);
					if (m_pPicking)
					{
						if (m_pick_drawed)
						{
							m_pick_reading = m_pPicking->readBlit(m_viewId[AgRenderPass::E_PASS_BLIT]);
							m_pick_drawed = false;
						}
						else if (m_pPicking->isPicked())
						{
							node->draw(m_viewId[AgRenderPass::E_PASS_ID],
								AgHardwarePickingSystem::getPickingShader(),
								AgHardwarePickingSystem::getPickingStates(),
								m_occusion_cull);
							m_pick_drawed = true;
						}
					}
				}
				else
				{
					node->draw(m_viewId[AgRenderPass::E_PASS_SHADING]);
				}
			}
		}

		if (m_pick_reading == m_currFrame)
		{
			m_renderQueueManager.m_queues[AgRenderQueueManager::E_WIREFRAME].destroy();
		}

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		m_currFrame = bgfx::frame();

	}
	
}