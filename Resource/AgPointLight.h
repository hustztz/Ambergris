#pragma once

#include "AgLight.h"

namespace ambergris {

	struct AgPointLight : public AgLight
	{
		enum TetrahedronFaces
		{
			E_TETRA_Green,
			E_TETRA_Yellow,
			E_TETRA_Blue,
			E_TETRA_Red,

			E_TETRA_Count
		};

		AgPointLight() : AgLight(), m_fovXAdjust(0.0f), m_fovYAdjust(0.0f), m_stencilPack(true) {

			m_position.m_x = m_position.m_y = m_position.m_z = 0.0f; m_position.m_w = 1.0f; //position
			m_diffusePower.m_power = 850.0f; //diffuse
			m_spotDirectionInner.m_x = 0.0f;
			m_spotDirectionInner.m_y = -0.4f;
			m_spotDirectionInner.m_z = -0.6f;
			m_spotDirectionInner.m_inner = 0.0f; //spotexponent
			m_attenuationSpotOuter.m_attnConst = 1.0f;
			m_attenuationSpotOuter.m_attnLinear = 0.0f;
			m_attenuationSpotOuter.m_attnQuadrantic = 1.0f;
			m_attenuationSpotOuter.m_outer = 91.0f; //attenuation, spotcutoff
		}

		virtual void prepareShadowMap(float** lightView, float** lightProj, bool isLinearDepth, uint16_t currentShadowMapSize) override;

		float m_fovXAdjust;
		float m_fovYAdjust;
		bool m_stencilPack;

	};
}