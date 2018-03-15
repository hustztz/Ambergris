#pragma once

#include "Foundation/Singleton.h"
#include "AgRenderPipeline.h"
#include "AgRenderQueue.h"
#include "AgRenderPass.h"

#include <map>

namespace ambergris {

	struct AgSceneDatabase;

	struct AgRenderer
	{
	public:
		void destroy();
		void clearNodes();
		void evaluateScene();
		void runPipeline();
		AgRenderNode::Handle appendNode(std::shared_ptr<AgRenderNode> renderNode, std::vector<AgResource::Handle>& objects);

		void updatePickingView(float* invViewProj, float mouseXNDC, float mouseYNDC, float fov, float nearFrusm, float farFrusm);
		void enableOcclusionQuery(bool enable);
		void enableHardwarePicking(bool enable);
		void enableSkySystem(bool enable) { m_pipeline.enableSkySystem(enable); }
		void updateTime(float time) { m_pipeline.updateTime(time); }

		bgfx::TextureHandle getDebugTexture() const { return m_pipeline.getDebugTexture(); }
	protected:
		void _UpdatePickingNodes();
	private:
		AgRenderer();
		~AgRenderer();
		friend class Singleton<AgRenderer>;
	public:
		std::atomic<bool>	m_isEvaluating;
		AgRenderPass		m_viewPass;
		AgRenderQueueManager	m_renderQueueManager;
	private:
		struct RenderHandle
		{
			RenderHandle(uint8_t _queue, AgRenderNode::Handle _node, uint8_t _item) : queue(_queue), node(_node), item(_item) {}
			uint8_t queue;
			AgRenderNode::Handle node;
			uint8_t	item;
		};
		typedef std::multimap<AgResource::Handle, RenderHandle> ObjectNodeMap;
		ObjectNodeMap		m_objectMap;
		AgRenderPipeline	m_pipeline;

		uint32_t		m_pick_reading;
		uint32_t		m_currFrame;
	};
}