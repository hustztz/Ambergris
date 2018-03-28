#include "AgRenderPipeline.h"
#include "AgHardwarePickingSystem.h"
#include "AgSkySystem.h"
#include "AgShadowSystem.h"
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
		m_fxSystem = new AgLightingSystem;
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
				AgSkySystem* skySystem = new AgSkySystem();
				if (!skySystem->init())
				{
					printf("Sky system failed to init.");
					delete skySystem;
					skySystem = nullptr;
				}
				m_fxSystem = skySystem;
			}
		}
		else
		{
			if (m_fxSystem)
			{
				if (typeid(*m_fxSystem) != typeid(AgSkySystem))
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
			}
		}
	}

	void AgRenderPipeline::enableShadow(bool enable)
	{
		if (enable)
		{
			if (m_fxSystem)
			{
				if (typeid(*m_fxSystem) == typeid(AgShadowSystem))
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
				AgShadowSystem::SmImpl::Enum	smImpl = AgShadowSystem::SmImpl::Hard;
				AgShadowSystem::DepthImpl::Enum depthImpl = AgShadowSystem::DepthImpl::InvZ;
				uint16_t shadowMapSize = 1024;
				AgLight* light = Singleton<AgRenderResourceManager>::instance().m_lights.get(0);//TODO
				AgMaterial* material = Singleton<AgRenderResourceManager>::instance().m_materials.get(AgMaterial::E_LAMBERT);//TODO
				bool blur = false;

				AgShadowSystem* shadowSystem = new AgShadowSystem();
				if (!shadowSystem->init(smImpl, depthImpl, shadowMapSize, light, material, blur))
				{
					printf("Failed to init shadow system.");
					delete shadowSystem;
				}
				m_fxSystem = shadowSystem;
			}
		}
		else
		{
			if (m_fxSystem)
			{
				if (typeid(*m_fxSystem) != typeid(AgShadowSystem))
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
		int32_t occlusionCulling)
	{
		if (m_fxSystem)
		{
			m_fxSystem->begin();
		}

		//Shadow pass
		AgShadowSystem* shadowSystem = dynamic_cast<AgShadowSystem*>(m_fxSystem);
		if (shadowSystem)
		{
			for (int j = 0; j < renderQueues.m_queues[AgRenderQueueManager::E_STATIC_SCENE_OPAQUE].getSize(); ++j)
			{
				const AgRenderNode* node = renderQueues.m_queues[AgRenderQueueManager::E_STATIC_SCENE_OPAQUE].get(j);
				if (!node || AgRenderNode::kInvalidHandle == node->m_handle)
					continue;

				shadowSystem->drawPackDepth(node);
			}
			shadowSystem->blurShadowMap();
			shadowSystem->prepareShadowMatrix();
		}

		// TODO: multithread
		for (int i = 0; i < AgRenderQueueManager::E_TYPE_COUNT; i ++)
		{
			if (AgRenderQueueManager::isScene((AgRenderQueueManager::QueueType)i))
			{
				for (int j = 0; j < renderQueues.m_queues[i].getSize(); ++j)
				{
					const AgRenderNode* node = renderQueues.m_queues[i].get(j);
					if (!node || AgRenderNode::kInvalidHandle == node->m_handle)
						continue;

					node->draw(allViews, m_fxSystem, occlusionCulling);

					if (m_pPicking && m_pPicking->isPicked())
					{
						ViewIdArray pickingView;
						pickingView.push_back(pickView);
						node->draw(pickingView, m_pPicking, -2/*occlusionCulling*/);//TODO:Picking is not support occlusion(crash)
						m_pick_drawed = true;
					}
				}
			}
		}

		if (m_fxSystem)
		{
			m_fxSystem->end();
		}

		// Wireframe or UI nodes
		for (int i = 0; i < renderQueues.m_queues[AgRenderQueueManager::E_WIREFRAME].getSize(); ++i)
		{
			const AgRenderNode* node = renderQueues.m_queues[AgRenderQueueManager::E_WIREFRAME].get(i);
			if (!node || AgRenderNode::kInvalidHandle == node->m_handle)
				continue;

			node->draw(mainView, m_wireframeSystem, -2/*occlusionCulling*/);
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