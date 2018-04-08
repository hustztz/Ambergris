#pragma once
#include "AgResourceContainer.h"
#include "AgRenderPass.h"
#include "Foundation/AgFrustum.h"
#include "Resource/AgBoundingBox.h"

#define VIEW_MAX_COUNT 2

namespace ambergris {

	struct AgCamera;

	class AgCameraView : public AgResource
	{
	public:
		AgCameraView() : m_camera(nullptr), m_x(0), m_y(0), m_width(0), m_height(0), m_pass(AgRenderPass::E_VIEW_COUNT), m_pointSize(2.0f){}

		void setCamera(AgCamera* _camera, AgRenderPass::RenderViewName _pass, uint32_t _width, uint32_t _height, uint32_t _x = 0, uint32_t _y = 0);
		const float* getViewMtx() const { return m_viewMtx; }
		const float* getProjMtx() const { return m_projMtx; }
		float getNearClip() const;
		float getFarClip() const;
		int calcLOD(AgBoundingbox::Handle voxelBounds, AgResource::Handle voxelTree) const;
		bool isRenderView() const {
			return m_pass >= AgRenderPass::E_VIEW_MAIN && m_pass <= AgRenderPass::RENDER_MAX_VIEW;
		}
		bool isMainView() const {	return m_pass == AgRenderPass::E_VIEW_MAIN;	}
		uint32_t getOffsetX() const { return m_x; }
		uint32_t getOffsetY() const { return m_y; }
		uint32_t getWidth() const { return m_width; }
		uint32_t getHeight() const { return m_height; }
		float getCameraFovy() const;
		bool isValid() const;

	protected:
		bool _CalcPixelSize(double* pixBounds, AgBoundingbox::Handle voxelBounds, AgResource::Handle voxelTree) const;
		void _SetFrustum();

	private:
		AgCamera* m_camera;
		uint32_t m_x;
		uint32_t m_y;
		uint32_t m_width;
		uint32_t m_height;
		AgRenderPass::RenderViewName m_pass;
		float m_viewMtx[16];
		float m_projMtx[16];
		AgFrustum m_frustum;

		float m_pointSize;
	};

	class AgCameraViewManager : public AgResourceContainer<AgCameraView, VIEW_MAX_COUNT>
	{
	public:
		size_t getValidSize() const;
	protected:
	private:
	};
}