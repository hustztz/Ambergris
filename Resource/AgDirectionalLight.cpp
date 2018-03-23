#include "AgDirectionalLight.h"

#include <bx/math.h>
#include <bgfx/bgfx.h>

namespace ambergris {

	void worldSpaceFrustumCorners(float* _corners24f
		, float _near
		, float _far
		, float _projWidth
		, float _projHeight
		, const float* __restrict _invViewMtx
	)
	{
		// Define frustum corners in view space.
		const float nw = _near * _projWidth;
		const float nh = _near * _projHeight;
		const float fw = _far  * _projWidth;
		const float fh = _far  * _projHeight;

		const uint8_t numCorners = 8;
		const float corners[numCorners][3] =
		{
			{ -nw,  nh, _near },
			{ nw,  nh, _near },
			{ nw, -nh, _near },
			{ -nw, -nh, _near },
			{ -fw,  fh, _far },
			{ fw,  fh, _far },
			{ fw, -fh, _far },
			{ -fw, -fh, _far },
		};

		// Convert them to world space.
		float(*out)[3] = (float(*)[3])_corners24f;
		for (uint8_t ii = 0; ii < numCorners; ++ii)
		{
			bx::vec3MulMtx((float*)&out[ii], (float*)&corners[ii], _invViewMtx);
		}
	}

	/**
	* _splits = { near0, far0, near1, far1... nearN, farN }
	* N = _numSplits
	*/
	void splitFrustum(float* _splits, uint8_t _numSplits, float _near, float _far, float _splitWeight = 0.75f)
	{
		const float l = _splitWeight;
		const float ratio = _far / _near;
		const int8_t numSlices = _numSplits * 2;
		const float numSlicesf = float(numSlices);

		// First slice.
		_splits[0] = _near;

		for (uint8_t nn = 2, ff = 1; nn < numSlices; nn += 2, ff += 2)
		{
			float si = float(int8_t(ff)) / numSlicesf;

			const float nearp = l*(_near*bx::pow(ratio, si)) + (1 - l)*(_near + (_far - _near)*si);
			_splits[nn] = nearp;          //near
			_splits[ff] = nearp * 1.005f; //far from previous split
		}

		// Last slice.
		_splits[numSlices - 1] = _far;
	}

	/*virtual*/
	void AgDirectionalLight::prepareShadowMap(float(*lightView)[16], float(*lightProj)[16], bool isLinearDepth, uint16_t currentShadowMapSize)
	{
		// Setup light view mtx.
		float eye[3] =
		{
			-m_position.m_x
			, -m_position.m_y
			, -m_position.m_z
		};
		float at[3] = { 0.0f, 0.0f, 0.0f };
		bx::mtxLookAt(lightView[0], eye, at);

		// Compute camera inverse view mtx.
		float mtxViewInv[16];
		bx::mtxInverse(mtxViewInv, m_viewMtx);

		// Compute split distances.
		BX_CHECK(s_maxNumSplits >= m_numSplits, "Error! Max num splits.");

		splitFrustum(m_splitSlices
			, uint8_t(m_numSplits)
			, m_near
			, m_far
			, m_splitDistribution
		);

		float mtxProj[16];
		bx::mtxOrtho(
			mtxProj
			, 1.0f
			, -1.0f
			, 1.0f
			, -1.0f
			, -m_far
			, m_far
			, 0.0f
			, bgfx::getCaps()->homogeneousDepth
		);

		const uint8_t numCorners = 8;
		float frustumCorners[s_maxNumSplits][numCorners][3];
		for (uint8_t ii = 0, nn = 0, ff = 1; ii < m_numSplits; ++ii, nn += 2, ff += 2)
		{
			// Compute frustum corners for one split in world space.
			worldSpaceFrustumCorners((float*)frustumCorners[ii], m_splitSlices[nn], m_splitSlices[ff], m_projWidth, m_projHeight, mtxViewInv);

			float min[3] = { 9000.0f,  9000.0f,  9000.0f };
			float max[3] = { -9000.0f, -9000.0f, -9000.0f };

			for (uint8_t jj = 0; jj < numCorners; ++jj)
			{
				// Transform to light space.
				float lightSpaceFrustumCorner[3];
				bx::vec3MulMtx(lightSpaceFrustumCorner, frustumCorners[ii][jj], lightView[0]);

				// Update bounding box.
				min[0] = bx::min(min[0], lightSpaceFrustumCorner[0]);
				max[0] = bx::max(max[0], lightSpaceFrustumCorner[0]);
				min[1] = bx::min(min[1], lightSpaceFrustumCorner[1]);
				max[1] = bx::max(max[1], lightSpaceFrustumCorner[1]);
				min[2] = bx::min(min[2], lightSpaceFrustumCorner[2]);
				max[2] = bx::max(max[2], lightSpaceFrustumCorner[2]);
			}

			float minproj[3];
			float maxproj[3];
			bx::vec3MulMtxH(minproj, min, mtxProj);
			bx::vec3MulMtxH(maxproj, max, mtxProj);

			float offsetx, offsety;
			float scalex, scaley;

			scalex = 2.0f / (maxproj[0] - minproj[0]);
			scaley = 2.0f / (maxproj[1] - minproj[1]);

			if (m_stabilize)
			{
				const float quantizer = 64.0f;
				scalex = quantizer / bx::ceil(quantizer / scalex);
				scaley = quantizer / bx::ceil(quantizer / scaley);
			}

			offsetx = 0.5f * (maxproj[0] + minproj[0]) * scalex;
			offsety = 0.5f * (maxproj[1] + minproj[1]) * scaley;

			if (m_stabilize)
			{
				const float halfSize = float(currentShadowMapSize) * 0.5f;
				offsetx = bx::ceil(offsetx * halfSize) / halfSize;
				offsety = bx::ceil(offsety * halfSize) / halfSize;
			}

			float mtxCrop[16];
			bx::mtxIdentity(mtxCrop);
			mtxCrop[0] = scalex;
			mtxCrop[5] = scaley;
			mtxCrop[12] = offsetx;
			mtxCrop[13] = offsety;

			bx::mtxMul(lightProj[ii], mtxCrop, mtxProj);
		}

		// Set render target
		float screenProj[16];
		float screenView[16];
		bx::mtxIdentity(screenView);

		bx::mtxOrtho(
			screenProj
			, 0.0f
			, 1.0f
			, 1.0f
			, 0.0f
			, 0.0f
			, 100.0f
			, 0.0f
			, bgfx::getCaps()->homogeneousDepth
		);
	}

	/*virtual*/ 
	void AgDirectionalLight::computeViewSpaceComponents(float* _viewMtx, float projWidth, float projHeight)
	{
		memcpy_s(m_viewMtx, 16 * sizeof(float), _viewMtx, 16 * sizeof(float));
		m_projHeight = projHeight;
		m_projWidth = projWidth;

		AgLight::computeViewSpaceComponents(_viewMtx, projWidth, projHeight);
	}
}