#include "AgRenderer.h"
#include "AgRenderSceneBridge.h"
#include "Scene/AgSceneDatabase.h"
#include "AgRenderProxyNode.h"
#include "AgRenderItem.h"
#include "Resource\AgRenderPass.h"
#include "Resource\AgRenderResourceManager.h"

#include "BGFX/entry/entry.h" //TODO

#include <thread>

#define OCCLUSION_QUERY_FREQ 1000

namespace ambergris {

	AgRenderer::AgRenderer()
		: m_isEvaluating(false)
		, m_pick_reading(0)
		, m_currFrame(UINT32_MAX)
		, m_occlusion_threshold(0)
		, m_activeView(AgCameraView::kInvalidHandle)
	{
		clearNodes();
	}

	AgRenderer::~AgRenderer()
	{
	}

	void AgRenderer::clearNodes()
	{
		m_renderQueueManager.destroy();
		m_geometryMapping.clear();
	}

	void AgRenderer::clearViews()
	{
		for (uint16_t ii = 0; ii < Singleton<AgRenderResourceManager>::instance().m_views.getSize(); ii++)
		{
			const AgCameraView* view = Singleton<AgRenderResourceManager>::instance().m_views.get(ii);
			if (!view)
				break;

			if (view->m_pass > AgRenderPass::RENDER_MAX_VIEW)
				break;

			if (AgCameraView::kInvalidHandle == view->m_handle)
				continue;

			bgfx::setViewClear(AgRenderPass::E_VIEW_MAIN + ii
				, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
				, 0x303030ff
				, 1.0f
				, 0
			);
		}
	}

	void AgRenderer::destroy()
	{
		clearNodes();
		m_pipeline.destroy();
	}

	void AgRenderer::updatePickingInfo(float mouseXNDC, float mouseYNDC)
	{
		const AgCameraView* view = Singleton<AgRenderResourceManager>::instance().m_views.get(m_activeView);
		if (!view)
			return;

		float viewMtx[16];
		view->getViewMtx(viewMtx);
		float projMtx[16];
		view->getProjMtx(projMtx);
		float viewProj[16];
		bx::mtxMul(viewProj, viewMtx, projMtx);
		float invViewProj[16];
		bx::mtxInverse(invViewProj, viewProj);
		float pickFovy = 3.0f; //TODO
		m_pipeline.updatePickingView(AgRenderPass::E_VIEW_ID, invViewProj, mouseXNDC, mouseYNDC, pickFovy, view->getNearClip(), view->getFarClip());
	}

	void AgRenderer::enableHardwarePicking(bool enable)
	{
		m_pipeline.enableHardwarePicking(enable);
	}

	void AgRenderer::enableOcclusionQuery(int32_t threshold)
	{
		//if (enable)
		//{
		//	m_viewPass.init(AgRenderPass::E_OCCLUSION_VIEW_MAIN);
		//	/*if()
		//	m_viewPass.init(AgRenderPass::E_OCCLUSION_VIEW_SECOND);*///TODO
		//}
		//else
		//{
		//	m_viewPass.m_pass_state[AgRenderPass::E_OCCLUSION_VIEW_MAIN].isValid = false;
		//	/*if()
		//	m_viewPass.m_pass_state[AgRenderPass::E_OCCLUSION_VIEW_SECOND].isValid = false;*///TODO
		//}
		m_occlusion_threshold = threshold;
	}

	void AgRenderer::evaluateScene()
	{
		if (Singleton<AgSceneDatabase>::instance().m_objectManager.m_dirty && !Singleton<AgRenderer>::instance().m_isEvaluating)
		{
#if BGFX_CONFIG_MULTITHREADED
			std::thread bridgeThread(AgRenderSceneBridge);
			bridgeThread.detach();
#else
			AgRenderSceneBridge();
#endif // BGFX_CONFIG_MULTITHREADED
		}
	}


	const AgRenderNode* AgRenderer::appendNode(std::shared_ptr<AgRenderNode> renderNode, AgGeometry::Handle geom)
	{
		if (!renderNode)
			return nullptr;

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
			AgRenderState renderState;
			renderState.m_state = 0
				| BGFX_STATE_WRITE_RGB
				| BGFX_STATE_WRITE_A
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_CULL_CCW
				| BGFX_STATE_MSAA;
			renderNode->setShaderState(renderState);
			nodeHandle = m_renderQueueManager.m_queues[AgRenderQueueManager::E_STATIC_SCENE_OPAQUE].append(renderNode);
			queue = AgRenderQueueManager::E_STATIC_SCENE_OPAQUE;
			
		}

		if (AgRenderNode::kInvalidHandle == nodeHandle)
			return nullptr;

		if(geom >= m_geometryMapping.size())
			m_geometryMapping.resize(geom + 1);
		m_geometryMapping[geom] = RenderHandle(queue, nodeHandle, 0);

		return getRenderNode(geom);
	}

	const AgRenderNode* AgRenderer::getRenderNode(AgGeometry::Handle geom) const
	{
		if (geom >= m_geometryMapping.size())
			return nullptr;
		const RenderHandle& renderHandle = m_geometryMapping.at(geom);
		return m_renderQueueManager.m_queues[renderHandle.queue].get(renderHandle.node);
	}

	const AgRenderer::RenderHandle& AgRenderer::getRenderHandle(AgGeometry::Handle geom) const
	{
		static AgRenderer::RenderHandle invalidRenderHandle = AgRenderer::RenderHandle();
		if (geom >= m_geometryMapping.size())
			return invalidRenderHandle;
		return m_geometryMapping.at(geom);
	}

	void AgRenderer::runPipeline()
	{
		_UpdateView();
		_UpdateLights();
		//Picking pass
		if (m_pick_reading == m_currFrame)
		{
			m_pick_reading = 0;
			uint8_t pick_result = m_pipeline.updatePickingResult();
			printf("Pick %d objects.\n", pick_result);
			_UpdatePickingNodes();
		}
		else if (!m_pick_reading)
		{
			m_pick_reading = m_pipeline.readPickingBlit(AgRenderPass::E_VIEW_BLIT);
		}

		ViewIdArray mainView;
		ViewIdArray allViews;
		for (uint16_t ii = 0; ii < Singleton<AgRenderResourceManager>::instance().m_views.getSize(); ii++)
		{
			const AgCameraView* view = Singleton<AgRenderResourceManager>::instance().m_views.get(ii);
			if (!view)
				break;

			if (view->m_pass > AgRenderPass::RENDER_MAX_VIEW)
				break;

			if (AgCameraView::kInvalidHandle == view->m_handle)
				continue;

			if (view->m_handle == m_activeView)
			{
				mainView.push_back(AgRenderPass::E_VIEW_MAIN + ii);
			}
			allViews.push_back(AgRenderPass::E_VIEW_MAIN + ii);
		}
		int32_t occlusionCulling = -2;
		if (m_occlusion_threshold > 0)
		{
			if (m_currFrame % OCCLUSION_QUERY_FREQ)
			{
				occlusionCulling = m_occlusion_threshold;
			}
			else
			{
				occlusionCulling = -1;
			}
		}
		m_currFrame = m_pipeline.run(m_renderQueueManager, mainView, allViews, AgRenderPass::E_VIEW_ID, occlusionCulling);
	}

	void AgRenderer::_UpdatePickingNodes()
	{
		m_renderQueueManager.m_queues[AgRenderQueueManager::E_WIREFRAME].destroy();
		const AgSceneDatabase& scene = Singleton<AgSceneDatabase>::instance();
		if (scene.m_select_result.empty())
			return;

		for (auto iter = scene.m_select_result.cbegin(); iter != scene.m_select_result.cend(); ++iter)
		{
			const AgMesh* pMesh = dynamic_cast<const AgMesh*>(scene.m_objectManager.get(*iter));
			if (!pMesh)
				continue;

			for (auto geom = pMesh->m_geometries.cbegin(); geom != pMesh->m_geometries.cend(); geom ++)
			{
				const AgGeometry* pGeometry = Singleton<AgRenderResourceManager>::instance().m_geometries.get(*geom);
				if (!pGeometry)
					continue;

				RenderHandle renderHandle = m_geometryMapping.at(pGeometry->m_handle);
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

	void AgRenderer::_UpdateView()
	{
		for (uint16_t ii = 0; ii < Singleton<AgRenderResourceManager>::instance().m_views.getSize(); ii++)
		{
			const AgCameraView* view = Singleton<AgRenderResourceManager>::instance().m_views.get(ii);
			if (!view)
				break;

			if(view->m_pass > AgRenderPass::RENDER_MAX_VIEW)
				break;

			if (view->m_pass == AgRenderPass::E_VIEW_MAIN)
				m_activeView = view->m_handle;

			if (AgCameraView::kInvalidHandle == view->m_handle)
				continue;

			float viewMtx[16];
			view->getViewMtx(viewMtx);
			float projMtx[16];
			view->getProjMtx(projMtx);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_MAIN + ii, viewMtx, projMtx);

			if (view->m_handle == m_activeView)
			{
				// Set view 0 default viewport.
				bgfx::setViewRect(AgRenderPass::E_VIEW_MAIN + ii, 0, 0, bgfx::BackbufferRatio::Equal);
			}
			else
			{
				bgfx::setViewRect(AgRenderPass::E_VIEW_MAIN + ii, uint16_t(view->m_x), uint16_t(view->m_y), uint16_t(view->m_width), uint16_t(view->m_height));
			}
			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(AgRenderPass::E_VIEW_MAIN + ii);
		}
	}

	void AgRenderer::_UpdateLights()
	{
		const AgCameraView* view = Singleton<AgRenderResourceManager>::instance().m_views.get(m_activeView);
		if (!view)
			return;

		float viewMtx[16];
		view->getViewMtx(viewMtx);
		const float camAspect = float(view->m_width) / float(view->m_height);
		const float lightFovy = view->m_camera.getFovy();//TODO

		const float projHeight = 1.0f / bx::tan(bx::toRad(lightFovy)*0.5f);
		const float projWidth = projHeight * camAspect;

		for (int ii = 0; ii < Singleton<AgRenderResourceManager>::instance().m_lights.getSize(); ii++)
		{
			AgLight* light = Singleton<AgRenderResourceManager>::instance().m_lights.get(ii);
			if(!light)
				continue;
			light->computeViewSpaceComponents(viewMtx, projWidth, projHeight);
		}
	}
}