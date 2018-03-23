#pragma once

#include "AgLight.h"

namespace ambergris {

	struct AgDirectionalLight : public AgLight
	{
		AgDirectionalLight() : AgLight(), m_splitDistribution(0.6f), m_numSplits(4), m_stabilize(true) {
			//position
			m_position.m_x = 0.5f;
			m_position.m_y = -1.0f;
			m_position.m_z = 0.1f;
			m_position.m_w = 0.0f;
			//ambient
			m_ambientPower.m_power = 0.02f;
			//diffuse
			m_diffusePower.m_power = 0.4f;
		}

		virtual void prepareShadowMap(float(*view)[16], float(*proj)[16], bool isLinearDepth, uint16_t currentShadowMapSize) override;
		virtual void computeViewSpaceComponents(float* _viewMtx, float projWidth, float projHeight) override;

		float	m_splitDistribution;
		int		m_numSplits;
		bool	m_stabilize;

		float   m_viewMtx[16];
		float	m_projWidth;
		float	m_projHeight;


		static const uint8_t s_maxNumSplits = 4;
		float m_splitSlices[s_maxNumSplits * 2];
	};
}