#include "AgRenderer.h"
#include "AgRenderSceneBridge.h"
#include "Scene/AgSceneDatabase.h"
#include "AgRenderProxyNode.h"
#include "AgRenderItem.h"

#include "BGFX/entry/entry.h" //TODO

#include <thread>

namespace ambergris {

	AgRenderer::AgRenderer()
		: m_isEvaluating(false)
		, m_pick_reading(0)
		, m_currFrame(UINT32_MAX)
	{
		clearNodes();
	}

	AgRenderer::~AgRenderer()
	{
	}

	void AgRenderer::clearNodes()
	{
		m_renderQueueManager.destroy();
		m_objectMap.clear();
	}

	void AgRenderer::destroy()
	{
		clearNodes();
		m_pipeline.destroy();
	}

	void AgRenderer::updatePickingView(float* invViewProj, float mouseXNDC, float mouseYNDC, float fov, float nearFrusm, float farFrusm)
	{
		m_pipeline.updatePickingView(AgRenderPass::E_PASS_ID, invViewProj, mouseXNDC, mouseYNDC, fov, nearFrusm, farFrusm);
	}

	void AgRenderer::enableHardwarePicking(bool enable)
	{
		if (enable)
		{
			m_viewPass.init(AgRenderPass::E_PASS_ID);
		}
		else
		{
			m_viewPass.m_pass_state[AgRenderPass::E_PASS_ID].isValid = false;
		}
		m_pipeline.enableHardwarePicking(enable);
	}

	void AgRenderer::enableOcclusionQuery(bool enable)
	{
		if (enable)
		{
			m_viewPass.init(AgRenderPass::E_OCCLUSION_VIEW_MAIN);
			/*if()
			m_viewPass.init(AgRenderPass::E_OCCLUSION_VIEW_SECOND);*///TODO
		}
		else
		{
			m_viewPass.m_pass_state[AgRenderPass::E_OCCLUSION_VIEW_MAIN].isValid = false;
			/*if()
			m_viewPass.m_pass_state[AgRenderPass::E_OCCLUSION_VIEW_SECOND].isValid = false;*///TODO
		}
		m_pipeline.enableOcclusionQuery(enable);
	}

	void AgRenderer::evaluateScene()
	{
		if (Singleton<AgSceneDatabase>::instance().m_dirty && !Singleton<AgRenderer>::instance().m_isEvaluating)
		{
#if BGFX_CONFIG_MULTITHREADED
			std::thread bridgeThread(AgRenderSceneBridge);
			bridgeThread.detach();
#else
			AgRenderSceneBridge();
#endif // BGFX_CONFIG_MULTITHREADED
		}
	}


	AgRenderNode::Handle AgRenderer::appendNode(std::shared_ptr<AgRenderNode> renderNode, std::vector<AgResource::Handle>& objects)
	{
		if (!renderNode)
			return AgRenderNode::kInvalidHandle;

		AgRenderNode::Handle nodeHandle = AgRenderNode::kInvalidHandle;
		//TODO
		uint8_t queue = AgRenderQueueManager::E_STATIC_SCENE_OPAQUE;
		if (renderNode->isTransparent())
		{
			nodeHandle = m_renderQueueManager.m_queues[AgRenderQueueManager::E_STATIC_SCENE_TRANSPARENT].append(renderNode);
			queue = AgRenderQueueManager::E_STATIC_SCENE_TRANSPARENT;
		}
		else
		{
			uint64_t state_flags = 0
				| BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_CULL_CCW
				| BGFX_STATE_MSAA;
			renderNode->setShader(AgShader::E_MESH_SHADING, state_flags);
			nodeHandle = m_renderQueueManager.m_queues[AgRenderQueueManager::E_STATIC_SCENE_OPAQUE].append(renderNode);
			queue = AgRenderQueueManager::E_STATIC_SCENE_OPAQUE;
			
		}

		if (AgRenderNode::kInvalidHandle == nodeHandle)
			return AgRenderNode::kInvalidHandle;

		if (AgRenderNode::kInvalidHandle == nodeHandle)
			return nodeHandle;

		uint8_t index = 0;
		for (auto object = objects.cbegin(); object != objects.cend(); object++, index++)
		{
			m_objectMap.insert(std::pair<AgResource::Handle, RenderHandle>(*object, RenderHandle(queue, nodeHandle, index)));
		}
		return nodeHandle;
	}

	void AgRenderer::runPipeline()
	{
		if (m_pick_reading == m_currFrame)
		{
			m_pick_reading = 0;
			uint8_t pick_result = m_pipeline.updatePickingResult();
			printf("Pick %d objects.\n", pick_result);
			_UpdatePickingNodes();
		}
		else if (!m_pick_reading)
		{
			m_pick_reading = m_pipeline.readPickingBlit(AgRenderPass::E_PASS_BLIT);
		}

		m_viewPass.clear();

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
		m_currFrame = m_pipeline.run(m_renderQueueManager, mainView, allViews, AgRenderPass::E_PASS_ID, occlusionViews);
	}

	void AgRenderer::_UpdatePickingNodes()
	{
		m_renderQueueManager.m_queues[AgRenderQueueManager::E_WIREFRAME].destroy();
		const AgSceneDatabase& scene = Singleton<AgSceneDatabase>::instance();
		if (scene.m_select_result.empty())
			return;

		for (auto iter = scene.m_select_result.cbegin(); iter != scene.m_select_result.cend(); ++iter)
		{
			const AgMesh* pMesh = dynamic_cast<const AgMesh*>(scene.get(*iter));
			if (!pMesh)
				continue;

			auto pick_result_beg = m_objectMap.lower_bound(pMesh->m_handle);
			auto pick_result_end = m_objectMap.upper_bound(pMesh->m_handle);
			for (auto pick_result = pick_result_beg; pick_result != pick_result_end; pick_result++)
			{
				RenderHandle renderHandle = pick_result->second;
				const AgRenderNode* node = m_renderQueueManager.m_queues[renderHandle.queue].get(renderHandle.node);
				if (node)
				{
					const AgRenderItem* pItem = node->getItem(renderHandle.item);
					if (pItem)
					{
						std::shared_ptr<AgRenderProxyNode> wireframeNode(BX_NEW(entry::getAllocator(), AgRenderProxyNode));
						if (wireframeNode)
						{
							const float* overrideTransform = node->getTransform(renderHandle.item);
							if (overrideTransform)
								wireframeNode->setTransform(overrideTransform);
							else
								wireframeNode->setTransform(pItem->m_mtx);
							wireframeNode->appendItem(pItem);
							m_renderQueueManager.m_queues[AgRenderQueueManager::E_WIREFRAME].append(wireframeNode);
						}
					}
				}
			}
		}
	}
}