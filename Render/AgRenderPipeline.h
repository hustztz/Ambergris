#pragma once

#include "AgRenderQueue.h"
#include "AgRenderPass.h"

namespace ambergris {

	class AgHardwarePickingSystem;

	class AgRenderPipeline
	{
	public:
		
		
		void reset();
		void run();
		void updatePickingInfo(float* invViewProj, float mouseXNDC, float mouseYNDC);
		AgRenderNode::Handle appendNode(AgRenderNode* renderNode);
	protected:
		AgRenderPipeline();
		~AgRenderPipeline();
		friend struct AgRenderer;
	private:
		bool			m_occusion_cull;
		bool			m_pick_drawed;
		uint32_t		m_pick_reading;
		uint32_t		m_currFrame;
		bgfx::ViewId    m_viewId[AgRenderPass::E_PASS_COUNT];
		AgHardwarePickingSystem*	m_pPicking;
		AgRenderQueueManager	m_renderQueueManager;
	};

}