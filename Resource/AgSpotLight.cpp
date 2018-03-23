#include "AgSpotLight.h"

#include <bx/math.h>
#include <bgfx/bgfx.h>

namespace ambergris {

	/*virtual*/
	void AgSpotLight::prepareShadowMap(float(*lightView)[16], float(*lightProj)[16], bool isLinearDepth, uint16_t currentShadowMapSize)
	{
		const float fovy = m_coverageSpotL;
		const float aspect = 1.0f;
		bx::mtxProj(
			lightProj[ProjType::Horizontal]
			, fovy
			, aspect
			, m_near
			, m_far
			, false
		);

		//For linear depth, prevent depth division by variable w-component in shaders and divide here by far plane
		if (isLinearDepth)
		{
			lightProj[ProjType::Horizontal][10] /= m_far;
			lightProj[ProjType::Horizontal][14] /= m_far;
		}

		float at[3];
		bx::vec3Add(at, m_position.m_v, m_spotDirectionInner.m_v);
		bx::mtxLookAt(lightView[ProjType::Horizontal], m_position.m_v, at);
	}
}