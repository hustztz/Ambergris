#include "AgRenderPipeline.h"
#include "AgHardwarePickingSystem.h"
#include "AgSkySystem.h"
#include "AgLightingSystem.h"
#include "AgOcclusionSystem.h"
#include "AgWireframeSystem.h"
#include "Resource/AgRenderResourceManager.h"

#include "AgRenderProxyNode.h"
#include "Scene/AgSceneDatabase.h"
#include "BGFX/entry/entry.h" //TODO

namespace ambergris {

	AgRenderPipeline::AgRenderPipeline(AgRenderPass&	pass)
		: m_occlusionSystem(nullptr)
		, m_pPicking(nullptr)
		, m_fxSystem(nullptr)
		, m_wireframeSystem(nullptr)
		, m_pick_reading(0)
		, m_pick_drawed(false)
		, m_currFrame(UINT32_MAX)
		, m_viewPass(pass)
	{
		reset();
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
		m_renderQueueManager.destroy();
		enableHardwarePicking(false);
		enableSkySystem(false);
	}

	AgRenderNode::Handle AgRenderPipeline::appendNode(std::shared_ptr<AgRenderNode> renderNode)
	{
		return m_renderQueueManager.appendNode(renderNode);
	}

	void AgRenderPipeline::updatePickingView(float* invViewProj, float mouseXNDC, float mouseYNDC, float fov, float nearFrusm, float farFrusm)
	{
		if(m_pPicking)
			m_pPicking->updateView(AgRenderPass::E_PASS_ID, invViewProj, mouseXNDC, mouseYNDC, fov, nearFrusm, farFrusm);
	}

	void AgRenderPipeline::enableHardwarePicking(bool enable)
	{
		if (enable)
		{
			if (!m_pPicking)
			{
				m_pPicking = new AgHardwarePickingSystem();
				m_pPicking->init();
				m_viewPass.init(AgRenderPass::E_PASS_ID);
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
			m_viewPass.m_pass_state[AgRenderPass::E_PASS_ID].isValid = false;
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
				m_viewPass.init(AgRenderPass::E_OCCLUSION_VIEW_MAIN);
				/*if()
					m_viewPass.init(AgRenderPass::E_OCCLUSION_VIEW_SECOND);*///TODO
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
				m_viewPass.m_pass_state[AgRenderPass::E_OCCLUSION_VIEW_MAIN].isValid = false;
				/*if()
				m_viewPass.m_pass_state[AgRenderPass::E_OCCLUSION_VIEW_SECOND].isValid = false;*///TODO
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
		m_viewPass.clear();

		if (m_pick_reading == m_currFrame)
		{
			m_pick_reading = 0;
			if (m_pPicking)
			{
				uint8_t nPicked = m_pPicking->acquireResult();
				if (nPicked)
				{
					m_renderQueueManager.m_queues[AgRenderQueueManager::E_WIREFRAME].destroy();
					// TODO: need to build a map to accelerate picking result
					const AgSceneDatabase& scene = Singleton<AgSceneDatabase>::instance();
					for (auto iter = scene.m_select_result.cbegin(); iter != scene.m_select_result.cend(); ++ iter)
					{
						for (int i = 0; i < AgRenderQueueManager::E_TYPE_COUNT; i++)
						{
							if (AgRenderQueueManager::isScene((AgRenderQueueManager::QueueType)i))
							{
								for (int j = 0; j < m_renderQueueManager.m_queues[i].getSize(); ++j)
								{
									const AgRenderNode* node = m_renderQueueManager.m_queues[i].get(j);
									if (!node || !node->m_dirty)
										continue;
									const AgMesh* pMesh = dynamic_cast<const AgMesh*>(scene.get(*iter));
									if (!pMesh)
										continue;
									const AgRenderItem* pItem = node->findItem(pMesh->m_pick_id);
									if (pItem)
									{
										std::shared_ptr<AgRenderProxyNode> wireframeNode(BX_NEW(entry::getAllocator(), AgRenderProxyNode));
										if (wireframeNode)
										{
											wireframeNode->appendItem(pItem);
											m_renderQueueManager.m_queues[AgRenderQueueManager::E_WIREFRAME].append(wireframeNode);
										}
									}
								}
							}
						}
					}
				}
				else
				{
					m_renderQueueManager.m_queues[AgRenderQueueManager::E_WIREFRAME].destroy();
				}
			}
		}

		ViewIdArray mainView;
		ViewIdArray allViews;
		ViewIdArray occlusionViews;
		for (uint16_t i = 0; i < AgRenderPass::E_PASS_COUNT; i++)
		{
			if (!m_viewPass.m_pass_state[i].isValid)
				continue;

			switch (i)
			{
			case AgRenderPass::E_VIEW_MAIN:
				mainView.push_back(i);
				allViews.push_back(i);
				break;
			case AgRenderPass::E_OCCLUSION_VIEW_MAIN:
			case AgRenderPass::E_OCCLUSION_VIEW_SECOND:
				occlusionViews.push_back(i);
				break;
			case AgRenderPass::E_VIEW_SECOND:
				allViews.push_back(i);
				break;
			default:
				break;
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
						node->draw(occlusionViews, m_occlusionSystem, false/*inOcclusion*/);
					node->draw(allViews, m_fxSystem, m_occlusionSystem ? true : false);
					if (m_pPicking)
					{
						if (m_pick_drawed && !m_pick_reading)
						{
							m_pick_reading = m_pPicking->readBlit(AgRenderPass::E_PASS_BLIT);
							m_pick_drawed = false;
						}
						else if (m_pPicking->isPicked())
						{
							ViewIdArray pickingView;
							pickingView.push_back(AgRenderPass::E_PASS_ID);
							node->draw(pickingView, m_pPicking, m_occlusionSystem ? true : false);
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
						node->draw(mainView, m_wireframeSystem, m_occlusionSystem ? true : false);
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
					node->draw(mainView, m_fxSystem, false/*inOcclusionQuery*/);
				}
			}
		}

		if (m_fxSystem)
		{
			m_fxSystem->auxiliaryDraw();
		}

		Singleton<AgRenderResourceManager>::instance().m_text.submit(AgRenderPass::E_VIEW_MAIN);

		// Advance to next frame. Rendering thread will be kicked to
		// process submitted rendering primitives.
		m_currFrame = bgfx::frame();
	}
	
}