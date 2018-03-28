#pragma once
#include "AgResourceContainer.h"
#include "AgCamera.h"
#include "AgRenderPass.h"

#define VIEW_MAX_COUNT 2

namespace ambergris {

	struct AgCameraView : public AgResource
	{
		AgCameraView() : m_x(0), m_y(0), m_width(0), m_height(0), m_pass(AgRenderPass::E_VIEW_COUNT) {}

		void getViewMtx(float* mtx) const;
		void getProjMtx(float* mtx) const;
		float getNearClip() const;
		float getFarClip() const;

		AgCamera m_camera;
		uint32_t m_x;
		uint32_t m_y;
		uint32_t m_width;
		uint32_t m_height;
		AgRenderPass::RenderViewName m_pass;
	};

	class AgCameraViewManager : public AgResourceContainer<AgCameraView, VIEW_MAX_COUNT>
	{
	public:
	protected:
	private:
	};
}