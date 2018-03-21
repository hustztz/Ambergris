#pragma once

#include "Foundation/Singleton.h"
#include "AgRenderPipeline.h"
#include "AgRenderQueue.h"
#include "AgRenderPass.h"
#include "Resource/AgGeometry.h"

namespace ambergris {

	struct AgSceneDatabase;

	struct AgRenderer
	{
	public:
		struct RenderHandle
		{
			RenderHandle(uint8_t _queue, AgRenderNode::Handle _node, uint8_t _item) : queue(_queue), node(_node), item(_item) {}
			RenderHandle() : queue(0), node(AgRenderNode::kInvalidHandle), item(0) {}

			uint8_t queue;
			AgRenderNode::Handle node;
			uint8_t	item;
		};
	public:
		void destroy();
		void clearNodes();
		void evaluateScene();
		void runPipeline();

		const AgRenderNode* appendNode(std::shared_ptr<AgRenderNode> renderNode, AgGeometry::Handle geom);
		const RenderHandle& getRenderHandle(AgGeometry::Handle geom) const;
		const AgRenderNode* getRenderNode(AgGeometry::Handle geom) const;

		void updatePickingView(float* invViewProj, float mouseXNDC, float mouseYNDC, float fov, float nearFrusm, float farFrusm);
		void enableOcclusionQuery(int32_t threshold);
		void enableHardwarePicking(bool enable);
		void enableSkySystem(bool enable) { m_pipeline.enableSkySystem(enable); }
		void updateTime(float time) { m_pipeline.updateTime(time); }
		void updateLights(float* view, float projWidth, float projHeight);

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
		typedef std::vector<RenderHandle> GeometryNodeMapping;
		GeometryNodeMapping	m_geometryMapping;
		AgRenderPipeline	m_pipeline;

		uint32_t		m_pick_reading;
		uint32_t		m_currFrame;

		int32_t			m_occlusion_threshold;
	};
}