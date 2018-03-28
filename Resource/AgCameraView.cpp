#include "AgCameraView.h"

#include <bx/math.h>

#define FAR_VIEW_CLIP 10000.0f
#define NEAR_VIEW_CLIP 0.1f

namespace ambergris {

	void AgCameraView::getViewMtx(float* mtx) const
	{
		if (!mtx)
			return;
		m_camera.getViewMtx(mtx);
	}

	void AgCameraView::getProjMtx(float* mtx) const
	{
		const float camAspect = float(m_width) / float(m_height);
		bx::mtxProj(mtx, m_camera.getFovy(), camAspect, NEAR_VIEW_CLIP, FAR_VIEW_CLIP, true);// bgfx::getCaps()->homogeneousDepth);
	}

	float AgCameraView::getNearClip() const
	{
		return NEAR_VIEW_CLIP;
	}

	float AgCameraView::getFarClip() const
	{
		return FAR_VIEW_CLIP;
	}
}