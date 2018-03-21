#pragma once
#include "AgLight.h"

namespace ambergris {

	struct AgSpotLight : public AgLight
	{
		AgSpotLight() : AgLight(), m_spotOuterAngle(45.0f), m_spotInnerAngle(30.0f), m_coverageSpotL(90.0f)
		{
			m_near = 1.0f;
			m_far = 250.0f;
		}

		virtual void prepareShadowMap(float** lightView, float** lightProj, bool isLinearDepth, uint16_t currentShadowMapSize) override;

		float m_spotOuterAngle;
		float m_spotInnerAngle;
		float m_coverageSpotL;
	};
}