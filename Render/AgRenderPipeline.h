#pragma once

#include <bgfx/bgfx.h>

#include <vector>

namespace ambergris {

	class AgHardwarePickingSystem;
	class AgFxSystem;
	class AgOcclusionSystem;
	class AgWireframeSystem;
	struct AgRenderQueueManager;

	typedef std::vector<bgfx::ViewId> ViewIdArray;

	class AgRenderPipeline
	{
	public:
		void destroy();
		uint32_t run(const AgRenderQueueManager& renderQueues,
			const ViewIdArray& mainView,
			const ViewIdArray& allViews,
			bgfx::ViewId		pickView,
			const ViewIdArray& occlusionViews);

		uint8_t updatePickingResult(bool isSinglePick = true);
		uint32_t readPickingBlit(bgfx::ViewId view_pass);
		void updatePickingView(bgfx::ViewId view_pass, float* invViewProj, float mouseXNDC, float mouseYNDC, float fov, float nearFrusm, float farFrusm);

		void updateTime(float time);
		void enableOcclusionQuery(bool enable);
		void enableHardwarePicking(bool enable);
		void enableSkySystem(bool enable);

		bgfx::TextureHandle getDebugTexture() const;
	protected:
		AgRenderPipeline();
		~AgRenderPipeline();
		friend struct AgRenderer;
	private:
		bool			m_pick_drawed;
		AgHardwarePickingSystem*	m_pPicking;
		AgFxSystem*				m_fxSystem;
		AgOcclusionSystem*		m_occlusionSystem;
		AgWireframeSystem*		m_wireframeSystem;
	};

}