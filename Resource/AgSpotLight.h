#pragma once
#include "AgLight.h"

namespace ambergris {

	struct AgSpotLight : public AgLight
	{
		AgSpotLight() : AgLight(), m_spotOuterAngle(45.0f), m_spotInnerAngle(30.0f), m_coverageSpotL(90.0f)
		{
			m_near = 1.0f;
			m_far = 250.0f;

			m_diffusePower.m_power = 850.0f;
			m_spotDirectionInner.m_x = 0.0f; // spotdirection
			m_spotDirectionInner.m_y = -0.4f; // spotdirection
			m_spotDirectionInner.m_z = -0.6f; // spotdirection
			m_attenuationSpotOuter.m_attnConst = 1.0f;
			m_attenuationSpotOuter.m_attnLinear = 0.0f;
			m_attenuationSpotOuter.m_attnQuadrantic = 1.0f;
			m_attenuationSpotOuter.m_outer = 91.0f;
		}

		virtual void prepareShadowMap(float(*view)[16], float(*proj)[16], bool isLinearDepth, uint16_t currentShadowMapSize) override;

		float m_spotOuterAngle;
		float m_spotInnerAngle;
		float m_coverageSpotL;
	};
}