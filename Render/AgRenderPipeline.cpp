#include "AgRenderPipeline.h"
#include "AgHardwarePickingSystem.h"
#include "AgSkySystem.h"
#include "AgLightingSystem.h"
#include "AgWireframeSystem.h"
#include "AgRenderQueue.h"
#include "Resource/AgRenderResourceManager.h"

namespace ambergris {

	AgRenderPipeline::AgRenderPipeline()
		: m_pPicking(nullptr)
		, m_fxSystem(nullptr)
		, m_wireframeSystem(nullptr)
		, m_pick_drawed(false)
	{
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
	}

	void AgRenderPipeline::updatePickingView(bgfx::ViewId view_pass, float* invViewProj, float mouseXNDC, float mouseYNDC, float fov, float nearFrusm, float farFrusm)
	{
		if(m_pPicking)
			m_pPicking->updateView(view_pass, invViewProj, mouseXNDC, mouseYNDC, fov, nearFrusm, farFrusm);
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

	uint8_t AgRenderPipeline::updatePickingResult(bool isSinglePick /*= true*/)
	{
		if (!m_pPicking)
			return 0;
		return m_pPicking->acquireResult(isSinglePick);
	}

	uint32_t AgRenderPipeline::readPickingBlit(bgfx::ViewId view_pass)
	{
		if (!m_pPicking || !m_pick_drawed)
			return 0;
		
		uint32_t pick_reading = m_pPicking->readBlit(view_pass);
		m_pPicking->endPick();
		m_pick_drawed = false;

		return pick_reading;
	}

	uint32_t AgRenderPipeline::run(
		const AgRenderQueueManager& renderQueues,
		const ViewIdArray& mainView,
		const ViewIdArray& allViews,
		bgfx::ViewId		pickView,
		bool occlusionQuery, bool occlusionCulling)
	{
		// TODO: multithread
		for (int i = 0; i < AgRenderQueueManager::E_TYPE_COUNT; i ++)
		{
			if (AgRenderQueueManager::isScene((AgRenderQueueManager::QueueType)i))
			{
				for (int j = 0; j < renderQueues.m_queues[i].getSize(); ++j)
				{
					const AgRenderNode* node = renderQueues.m_queues[i].get(j);
					if (!node || !node->m_dirty)
						continue;

					//if(m_occlusionSystem)
					//	node->draw(occlusionViews, m_occlusionSystem, false/*inOcclusion*/);
					node->draw(allViews, m_fxSystem, occlusionQuery, occlusionCulling);
					if (m_pPicking && m_pPicking->isPicked())
					{
						ViewIdArray pickingView;
						pickingView.push_back(pickView);
						node->draw(pickingView, m_pPicking, false/*occlusionQuery*/, false/*occlusionCulling*/);//TODO:Picking is not support occlusion(crash)
						m_pick_drawed = true;
					}
				}
			}
		}

		if (m_fxSystem)
		{
			m_fxSystem->auxiliaryDraw();
		}

		// Wireframe or UI nodes
		for (int i = 0; i < renderQueues.m_queues[AgRenderQueueManager::E_WIREFRAME].getSize(); ++i)
		{
			const AgRenderNode* node = renderQueues.m_queues[AgRenderQueueManager::E_WIREFRAME].get(i);
			if (!node || !node->m_dirty)
				continue;

			node->draw(mainView, m_wireframeSystem, false/*occlusionQuery*/, false/*occlusionCulling*/);
		}

		for (ViewIdArray::const_iterator view = mainView.cbegin(), viewEnd = mainView.cend(); view != viewEnd; view++)
		{
			Singleton<AgRenderResourceManager>::instance().m_text.submit(*view);
		}
		

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		return bgfx::frame();
	}
	
}