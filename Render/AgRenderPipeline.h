#pragma once

#include "AgRenderQueue.h"
#include "AgRenderPass.h"

namespace ambergris {

	class AgHardwarePickingSystem;
	class AgFxSystem;
	class AgOcclusionSystem;
	class AgWireframeSystem;

	class AgRenderPipeline
	{
	public:
		void destroy();
		void reset();
		void run();
		void updatePickingInfo(float* invViewProj, float mouseXNDC, float mouseYNDC, float fov);
		void enableOcclusionQuery(bool enable);
		void enableHardwarePicking(bool enable);
		void enableSkySystem(bool enable);
		void updateTime(float time);
		AgRenderNode::Handle appendNode(AgRenderNode* renderNode);

		bgfx::TextureHandle getDebugTexture() const;
	protected:
		AgRenderPipeline();
		~AgRenderPipeline();
		friend struct AgRenderer;
	private:
		bool			m_pick_drawed;
		uint32_t		m_pick_reading;
		uint32_t		m_currFrame;
		bgfx::ViewId    m_viewId[AgRenderPass::E_PASS_COUNT];
		AgHardwarePickingSystem*	m_pPicking;
		AgFxSystem*				m_fxSystem;
		AgOcclusionSystem*		m_occlusionSystem;
		AgRenderQueueManager	m_renderQueueManager;
		AgWireframeSystem*		m_wireframeSystem;
	};

}