#include "AgCameraView.h"
#include "AgRenderResourceManager.h"
#include "Scene/AgCamera.h"
#include "Scene/AgSceneDatabase.h"
#include "Scene/PointCloud/AgVoxelTreeRunTime.h"

#include <algorithm>
#include <bx/math.h>

#include <common/RCTransform.h>

#define FAR_VIEW_CLIP 10000.0f
#define NEAR_VIEW_CLIP 0.1f

using namespace ambergris::RealityComputing::Common;

namespace ambergris {

	void AgCameraView::setCamera(AgCamera* _camera, AgRenderPass::RenderViewName _pass, uint32_t _width, uint32_t _height, uint32_t _x /*= 0*/, uint32_t _y /*= 0*/)
	{
		m_x = _x;
		m_y = _y;
		m_width = _width;
		m_height = _height;
		m_pass = _pass;

		if (_camera)
		{
			m_camera = _camera;

			m_camera->getViewMtx(m_viewMtx);

			const float camAspect = float(m_width) / float(m_height);
			bx::mtxProj(m_projMtx, m_camera->getFovy(), camAspect, NEAR_VIEW_CLIP, FAR_VIEW_CLIP, true);// bgfx::getCaps()->homogeneousDepth);

			_SetFrustum();
		}
	}

	void AgCameraView::_SetFrustum()
	{
		if (!m_camera)
			return;

		RCProjection projMVMatrix;
		projMVMatrix.fromColumnMajor(m_projMtx);
		m_frustum.setFrustum(projMVMatrix);
	}

	float AgCameraView::getNearClip() const
	{
		return NEAR_VIEW_CLIP;
	}

	float AgCameraView::getFarClip() const
	{
		return FAR_VIEW_CLIP;
	}

	float AgCameraView::getCameraFovy() const
	{
		if (!m_camera)
			return 0.0f;

		return m_camera->getFovy();
	}

	bool AgCameraView::_CalcPixelSize(double* pixBounds, AgBoundingbox::Handle voxelBounds, AgVoxelTreeRunTime::Handle voxelTreeHandle) const
	{
		if (!pixBounds)
			return false;

		// project the corners of the box into NDC screen space, and keep track of the 2D bounds
		const AgBoundingbox* bound = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(voxelBounds);
		if (!bound)
			return false;

		const AgVoxelTreeRunTime* voxelTree = Singleton<AgSceneDatabase>::instance().m_pointCloudManager.getPointCloudProject().get(voxelTreeHandle);
		if (!voxelTree)
			return false;

		float combinedMtx[16];
		bx::mtxMul(combinedMtx, m_viewMtx, m_projMtx);
		
		RCVector3d voxCorners[8];
		getCorners(bound->m_bounds, voxCorners);

		const RCPlane& nearPlane = m_frustum.getPlaneAt(AgFrustum::FRUST_NEAR);
		bool anyBehind = false;
		RCVector3d cornerWS;
		RCBox ndcBounds;
		for (int i = 0; i != 8; ++i)
		{
			RCVector3d cornerMS = voxCorners[i];
			voxelTree->transformPoint(cornerMS, cornerWS);

			// If part of the voxel is on the camera side of the near plane, then
			// a number of problems can occur:
			// 1. The "size" of the voxel could be infinite, if a corner is near 
			//    the camera's position.
			// 2. The shape of the voxel will be twisted if some of the corners 
			//    are behind the camera.
			// 3. If a corner is between the camera and the near plane, then
			//    part of the voxel will be clipped, and (for perspective projections) 
			//    the voxel's pixel size could be much larger than the region that 
			//    the actual (non-clipped) points occupy.
			//
			// A solution to 1 and 2 (and approximate solution for 3) 
			// is to "deform" the voxel so that it all fits in the NDC cube. 
			// This removes all degenerate cases (i.e. 1 and 2). The resulting 
			// pixel size might still be a bit larger than it should be, 
			// but it will be better than not deforming it. The below code does 
			// the deformation by just ensuring that each corner is behind the 
			// near plane, or by projecting it to the plane if the corner is 
			// in front of it.
			if (nearPlane.distanceToPlane(cornerWS) < 0)
			{
				cornerWS = nearPlane.projectionInPlane(cornerWS);
			}
			else
			{
				anyBehind = true;
			}

			// Transform world coordinates to NDC screen space
			RCVector3d cornerSS;
			cornerSS[0] = cornerWS[0] * (double)combinedMtx[0] + cornerWS[1] * (double)combinedMtx[4] + cornerWS[2] * (double)combinedMtx[8] + (double)combinedMtx[12];
			cornerSS[1] = cornerWS[0] * (double)combinedMtx[1] + cornerWS[1] * (double)combinedMtx[5] + cornerWS[2] * (double)combinedMtx[9] + (double)combinedMtx[13];
			cornerSS[2] = cornerWS[0] * (double)combinedMtx[2] + cornerWS[1] * (double)combinedMtx[6] + cornerWS[2] * (double)combinedMtx[10] + (double)combinedMtx[14];
			double w = cornerWS[0] * (double)combinedMtx[3] + cornerWS[1] * (double)combinedMtx[7] + cornerWS[2] * (double)combinedMtx[11] + cornerWS[3] * (double)combinedMtx[15];
			cornerSS[0] /= w;
			cornerSS[1] /= w;
			cornerSS[2] /= w;
			ndcBounds.updateBounds(cornerSS);
		}

		// See if any points were behind the near plane. 
		// If all points were in front, then the voxel is completely clipped.
		if (!anyBehind)
			return false;

		// make sure that part of the voxel is visible w.r.t the other clipping planes
		bool outside = false;
		outside |= ndcBounds.getMax().x < -1;
		outside |= ndcBounds.getMax().y < -1;
		outside |= ndcBounds.getMax().z < -1;
		outside |= ndcBounds.getMin().x > +1;
		outside |= ndcBounds.getMin().y > +1;
		outside |= ndcBounds.getMin().z > +1;
		if (outside)
		{
			return false;
		}

		// Convert NDC space to pixel space.
		RCVector3d ndcSpan = ndcBounds.getMax() - ndcBounds.getMin();
		pixBounds[0] = ndcSpan.x * m_width  * 0.5;
		pixBounds[1] = ndcSpan.y * m_height * 0.5;
		return true;
	}

	int AgCameraView::calcLOD(AgBoundingbox::Handle voxelBounds, AgVoxelTreeRunTime::Handle voxelTree) const
	{
		if (!m_camera)
			return 0;

		double pixBounds[2];
		if (!_CalcPixelSize(pixBounds, voxelBounds, voxelTree))
			return 0;
		double pixSize = std::max(pixBounds[0], pixBounds[1]);
		if (pixSize <= 0)
		{
			return 0;
		}

		// Check for infinity
		if (pixSize > std::numeric_limits<double>::max())
		{
			return std::numeric_limits<int>::max();
		}

		// Calc the actual LOD
		int lodLevel = 1;
		while (pixSize > m_pointSize)
		{
			pixSize *= 0.5f;
			lodLevel++;
		}

		return lodLevel;
	}

	bool AgCameraView::isValid() const
	{
		return AgCameraView::kInvalidHandle != m_handle && m_camera && m_pass < AgRenderPass::E_VIEW_COUNT;
	}

	size_t AgCameraViewManager::getValidSize() const
	{
		size_t num = 0;
		for (uint16_t ii = 0; ii < getSize(); ii ++)
		{
			const AgCameraView* view = get(ii);
			if (view && AgCameraView::kInvalidHandle != view->m_handle && view->isValid())
				num ++;
		}
		return num;
	}
}