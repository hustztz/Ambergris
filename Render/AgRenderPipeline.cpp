#include "AgRenderPipeline.h"
#include "AgHardwarePickingSystem.h"
#include "AgSkySystem.h"
#include "AgLightingSystem.h"
#include "AgOcclusionSystem.h"
#include "AgWireframeSystem.h"
#include "Resource/AgRenderResourceManager.h"

namespace ambergris {

	AgRenderPipeline::AgRenderPipeline()
		: m_occlusionSystem(nullptr)
		, m_pPicking(nullptr)
		, m_fxSystem(nullptr)
		, m_wireframeSystem(nullptr)
		, m_pick_reading(0)
		, m_pick_drawed(false)
		, m_currFrame(UINT32_MAX)
	{
		for (uint8_t i = 0; i < AgRenderPass::E_PASS_COUNT; i ++)
		{
			m_viewId[i] = i;
		}
		m_fxSystem = new AgLightingSystem();
		m_renderQueueManager.destroy();
	}

	AgRenderPipeline::~AgRenderPipeline()
	{
		destroy();
	}

	void AgRenderPipeline::destroy()
	{
		if (m_pPicking)
		{
			delete m_pPicking;
			m_pPicking = nullptr;
		}
		if (m_fxSystem)
		{
			delete m_fxSystem;
			m_fxSystem = nullptr;
		}
		if (m_wireframeSystem)
		{
			delete m_wireframeSystem;
			m_wireframeSystem = nullptr;
		}
		if (m_occlusionSystem)
		{
			delete m_occlusionSystem;
			m_occlusionSystem = nullptr;
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
		enableHardwarePicking(false);
		enableSkySystem(false);
	}

	AgRenderNode::Handle AgRenderPipeline::appendNode(AgRenderNode* renderNode)
	{
		return m_renderQueueManager.appendNode(renderNode);
	}

	void AgRenderPipeline::updatePickingInfo(float* invViewProj, float mouseXNDC, float mouseYNDC, float fov)
	{
		if(m_pPicking)
			m_pPicking->update(AgRenderPass::E_PASS_ID, invViewProj, mouseXNDC, mouseYNDC, fov);
	}

	void AgRenderPipeline::enableHardwarePicking(bool enable)
	{
		if (enable)
		{
			if (!m_pPicking)
			{
				m_pPicking = new AgHardwarePickingSystem();
				m_pPicking->init();
			}
			if (!m_wireframeSystem)
			{
				m_wireframeSystem = new AgWireframeSystem();
				m_wireframeSystem->init();
			}
		}
		else
		{
			if (m_pPicking)
			{
				delete m_pPicking;
				m_pPicking = nullptr;
			}
			if (m_wireframeSystem)
			{
				delete m_wireframeSystem;
				m_wireframeSystem = nullptr;
			}
		}
	}

	void AgRenderPipeline::enableOcclusionQuery(bool enable)
	{
		if (enable)
		{
			if (m_occlusionSystem)
			{
				return;
			}
			else
			{
				m_occlusionSystem = new AgOcclusionSystem();
				m_occlusionSystem->init();
			}
		}
		else
		{
			if (!m_occlusionSystem)
			{
				return;
			}
			else
			{
				m_occlusionSystem->destroy();
				delete m_occlusionSystem;
				m_occlusionSystem = nullptr;
			}
		}
	}

	void AgRenderPipeline::enableSkySystem(bool enable)
	{
		if (enable)
		{
			if (m_fxSystem)
			{
				if (typeid(*m_fxSystem) == typeid(AgSkySystem))
				{
					return;
				}
				else
				{
					if (m_fxSystem)
					{
						delete m_fxSystem;
						m_fxSystem = nullptr;
					}
				}
			}
			if (!m_fxSystem)
			{
				m_fxSystem = new AgSkySystem();
				if (!m_fxSystem->init())
				{
					printf("Sky system failed to init.");
					delete m_fxSystem;
					m_fxSystem = nullptr;
				}
			}
		}
		else
		{
			if (m_fxSystem)
			{
				if (typeid(*m_fxSystem) == typeid(AgLightingSystem))
				{
					return;
				}
				else
				{
					if (m_fxSystem)
					{
						delete m_fxSystem;
						m_fxSystem = nullptr;
					}
				}
			}
			if (!m_fxSystem)
			{
				m_fxSystem = new AgLightingSystem();
				if (!m_fxSystem->init())
				{
					delete m_fxSystem;
					m_fxSystem = nullptr;
				}
			}
		}
	}

	void AgRenderPipeline::updateTime(float time)
	{
		m_fxSystem->updateTime(time);
	}

	bgfx::TextureHandle AgRenderPipeline::getDebugTexture() const
	{
		if (!m_pPicking)
			return BGFX_INVALID_HANDLE;

		return m_pPicking->getFBTexture();
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

		if (m_pick_reading == m_currFrame)
		{
			m_pick_reading = 0;
			if (m_pPicking)
			{
				uint8_t nPicked = m_pPicking->acquireResult();
				if (nPicked)
				{
					m_renderQueueManager.m_queues[AgRenderQueueManager::E_WIREFRAME].destroy();
				}
			}
		}

		// TODO: multithread
		for (int i = 0; i < AgRenderQueueManager::E_TYPE_COUNT; i ++)
		{
			if (AgRenderQueueManager::isScene((AgRenderQueueManager::QueueType)i))
			{
				for (int j = 0; j < m_renderQueueManager.m_queues[i].getSize(); ++j)
				{
					AgRenderNode* node = m_renderQueueManager.m_queues[i].get(j);
					if (!node || !node->m_dirty)
						continue;

					if(m_occlusionSystem)
						node->draw(m_viewId[AgRenderPass::E_PASS_SHADING], m_occlusionSystem, false/*inOcclusion*/);
					node->draw(m_viewId[AgRenderPass::E_PASS_SHADING], m_fxSystem, m_occlusionSystem ? true : false);
					if (m_pPicking)
					{
						if (m_pick_drawed && !m_pick_reading)
						{
							m_pick_reading = m_pPicking->readBlit(m_viewId[AgRenderPass::E_PASS_BLIT]);
							m_pick_drawed = false;
						}
						else if (m_pPicking->isPicked())
						{
							node->draw(m_viewId[AgRenderPass::E_PASS_ID], m_pPicking, m_occlusionSystem ? true : false);
							m_pick_drawed = true;
						}
					}
				}
			}
			else if(AgRenderQueueManager::E_WIREFRAME == i)
			{
				if (m_wireframeSystem)
				{
					// Wireframe or UI nodes
					for (int j = 0; j < m_renderQueueManager.m_queues[i].getSize(); ++j)
					{
						AgRenderNode* node = m_renderQueueManager.m_queues[i].get(j);
						if (!node || !node->m_dirty)
							continue;
						node->draw(m_viewId[AgRenderPass::E_PASS_SHADING], m_wireframeSystem, m_occlusionSystem ? true : false);
					}
				}
			}
			else
			{
				for (int j = 0; j < m_renderQueueManager.m_queues[i].getSize(); ++j)
				{
					AgRenderNode* node = m_renderQueueManager.m_queues[i].get(j);
					if (!node || !node->m_dirty)
						continue;
					node->draw(m_viewId[AgRenderPass::E_PASS_SHADING], m_fxSystem, false/*inOcclusionQuery*/);
				}
			}
		}

		if (m_fxSystem)
		{
			m_fxSystem->auxiliaryDraw();
		}

		Singleton<AgRenderResourceManager>::instance().m_text.submit(m_viewId[AgRenderPass::E_PASS_SHADING]);

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		m_currFrame = bgfx::frame();
	}
	
}