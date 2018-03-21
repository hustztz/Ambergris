#include "AgPointLight.h"

#include <bx/math.h>
#include <bgfx/bgfx.h>

namespace ambergris {

	void mtxYawPitchRoll(float* __restrict _result
		, float _yaw
		, float _pitch
		, float _roll
	)
	{
		float sroll = bx::sin(_roll);
		float croll = bx::cos(_roll);
		float spitch = bx::sin(_pitch);
		float cpitch = bx::cos(_pitch);
		float syaw = bx::sin(_yaw);
		float cyaw = bx::cos(_yaw);

		_result[0] = sroll * spitch * syaw + croll * cyaw;
		_result[1] = sroll * cpitch;
		_result[2] = sroll * spitch * cyaw - croll * syaw;
		_result[3] = 0.0f;
		_result[4] = croll * spitch * syaw - sroll * cyaw;
		_result[5] = croll * cpitch;
		_result[6] = croll * spitch * cyaw + sroll * syaw;
		_result[7] = 0.0f;
		_result[8] = cpitch * syaw;
		_result[9] = -spitch;
		_result[10] = cpitch * cyaw;
		_result[11] = 0.0f;
		_result[12] = 0.0f;
		_result[13] = 0.0f;
		_result[14] = 0.0f;
		_result[15] = 1.0f;
	}

	/*virtual*/ void AgPointLight::prepareShadowMap(float** lightView, float** lightProj, bool isLinearDepth, uint16_t currentShadowMapSize)
	{
		float mtxYpr[TetrahedronFaces::E_TETRA_Count][16];

		float ypr[TetrahedronFaces::E_TETRA_Count][3] =
		{
			{ bx::toRad(0.0f), bx::toRad(27.36780516f), bx::toRad(0.0f) },
			{ bx::toRad(180.0f), bx::toRad(27.36780516f), bx::toRad(0.0f) },
			{ bx::toRad(-90.0f), bx::toRad(-27.36780516f), bx::toRad(0.0f) },
			{ bx::toRad(90.0f), bx::toRad(-27.36780516f), bx::toRad(0.0f) },
		};


		if (m_stencilPack)
		{
			const float fovx = 143.98570868f + 3.51f + m_fovXAdjust;
			const float fovy = 125.26438968f + 9.85f + m_fovYAdjust;
			const float aspect = bx::tan(bx::toRad(fovx*0.5f)) / bx::tan(bx::toRad(fovy*0.5f));

			bx::mtxProj(
				lightProj[ProjType::E_PROJ_Vertical]
				, fovx
				, aspect
				, m_near
				, m_far
				, false
			);

			//For linear depth, prevent depth division by variable w-component in shaders and divide here by far plane
			if (isLinearDepth)
			{
				lightProj[ProjType::E_PROJ_Vertical][10] /= m_far;
				lightProj[ProjType::E_PROJ_Vertical][14] /= m_far;
			}

			ypr[TetrahedronFaces::E_TETRA_Green][2] = bx::toRad(180.0f);
			ypr[TetrahedronFaces::E_TETRA_Yellow][2] = bx::toRad(0.0f);
			ypr[TetrahedronFaces::E_TETRA_Blue][2] = bx::toRad(90.0f);
			ypr[TetrahedronFaces::E_TETRA_Red][2] = bx::toRad(-90.0f);
		}

		const float fovx = 143.98570868f + 7.8f + m_fovXAdjust;
		const float fovy = 125.26438968f + 3.0f + m_fovYAdjust;
		const float aspect = bx::tan(bx::toRad(fovx*0.5f)) / bx::tan(bx::toRad(fovy*0.5f));

		bx::mtxProj(
			lightProj[ProjType::E_PROJ_Horizontal]
			, fovy
			, aspect
			, m_near
			, m_far
			, bgfx::getCaps()->homogeneousDepth
		);

		//For linear depth, prevent depth division by variable w component in shaders and divide here by far plane
		if (isLinearDepth)
		{
			lightProj[ProjType::E_PROJ_Horizontal][10] /= m_far;
			lightProj[ProjType::E_PROJ_Horizontal][14] /= m_far;
		}


		for (uint8_t ii = 0; ii < TetrahedronFaces::E_TETRA_Count; ++ii)
		{
			float mtxTmp[16];
			mtxYawPitchRoll(mtxTmp, ypr[ii][0], ypr[ii][1], ypr[ii][2]);

			float tmp[3] =
			{
				-bx::vec3Dot(m_position.m_v, &mtxTmp[0]),
				-bx::vec3Dot(m_position.m_v, &mtxTmp[4]),
				-bx::vec3Dot(m_position.m_v, &mtxTmp[8]),
			};

			bx::mtxTranspose(mtxYpr[ii], mtxTmp);

			bx::memCopy(lightView[ii], mtxYpr[ii], 12 * sizeof(float));
			lightView[ii][12] = tmp[0];
			lightView[ii][13] = tmp[1];
			lightView[ii][14] = tmp[2];
			lightView[ii][15] = 1.0f;
		}
	}
}