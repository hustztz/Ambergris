#include "AgSpotLight.h"

#include <bx/math.h>
#include <bgfx/bgfx.h>

namespace ambergris {

	/*virtual*/
	void AgSpotLight::prepareShadowMap(float** lightView, float** lightProj, bool isLinearDepth, uint16_t currentShadowMapSize)
	{
		const float fovy = m_coverageSpotL;
		const float aspect = 1.0f;
		bx::mtxProj(
			lightProj[E_PROJ_Horizontal]
			, fovy
			, aspect
			, m_near
			, m_far
			, false
		);

		//For linear depth, prevent depth division by variable w-component in shaders and divide here by far plane
		if (isLinearDepth)
		{
			lightProj[E_PROJ_Horizontal][10] /= m_far;
			lightProj[E_PROJ_Horizontal][14] /= m_far;
		}

		float at[3];
		bx::vec3Add(at, m_position.m_v, m_spotDirectionInner.m_v);
		bx::mtxLookAt(lightView[E_PROJ_Horizontal], m_position.m_v, at);
	}
}