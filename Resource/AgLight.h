#pragma once

#include "AgResourcePool.h"

#include <bx/math.h>

namespace ambergris {

	struct AgLight : public AgResource
	{
		enum LightType
		{
			E_SpotLight,
			E_PointLight,
			E_DirectionalLight,

			E_COUNT
		};

		enum ProjType
		{
			E_PROJ_Horizontal,
			E_PROJ_Vertical,

			E_PROJ_Count
		};

		union Position
		{
			struct
			{
				float m_x;
				float m_y;
				float m_z;
				float m_w;
			};

			float m_v[4];
		};

		union LightRgbPower
		{
			struct
			{
				float m_r;
				float m_g;
				float m_b;
				float m_power;
			};

			float m_v[4];
		};

		union SpotDirectionInner
		{
			struct
			{
				float m_x;
				float m_y;
				float m_z;
				float m_inner;
			};

			float m_v[4];
		};

		union AttenuationSpotOuter
		{
			struct
			{
				float m_attnConst;
				float m_attnLinear;
				float m_attnQuadrantic;
				float m_outer;
			};

			float m_v[4];
		};

		AgLight() : AgResource() {
			m_position.m_x = m_position.m_y = m_position.m_z = 0.0f; m_position.m_w = 1.0f;
			m_position_viewSpace[0] = m_position_viewSpace[1] = m_position_viewSpace[2] = m_position_viewSpace[3] = 0.0f;
			m_ambientPower.m_r = m_ambientPower.m_g = m_ambientPower.m_b = 1.0f; m_ambientPower.m_power = 0.0f; //ambient
			m_diffusePower.m_r = m_diffusePower.m_g = m_diffusePower.m_b = 1.0f; m_diffusePower.m_power = 1.0f; //diffuse
			m_specularPower.m_r = m_specularPower.m_g = m_specularPower.m_b = 1.0f; m_specularPower.m_power = 0.0f; //specular
			m_spotDirectionInner.m_x = m_spotDirectionInner.m_y = m_spotDirectionInner.m_z = 0.0; // spotdirection
			m_spotDirectionInner.m_inner = 1.0f; //spotexponent
			m_spotDirectionInner_viewSpace[0] = m_spotDirectionInner_viewSpace[1] = m_spotDirectionInner_viewSpace[2] = m_spotDirectionInner_viewSpace[3] = 0.0f;
			m_attenuationSpotOuter.m_attnConst = m_attenuationSpotOuter.m_attnLinear = m_attenuationSpotOuter.m_attnQuadrantic = 0.0f; m_attenuationSpotOuter.m_outer = 1.0f; //attenuation, spotcutoff
		}

		virtual ~AgLight(){}
		virtual void prepareShadowMap(float** view, float** proj, bool isLinearDepth, uint16_t currentShadowMapSize) {}
		virtual void computeViewSpaceComponents(float* _viewMtx, float projWidth, float projHeight)
		{
			bx::vec4MulMtx(m_position_viewSpace, m_position.m_v, _viewMtx);

			float tmp[] =
			{
				m_spotDirectionInner.m_x
				, m_spotDirectionInner.m_y
				, m_spotDirectionInner.m_z
				, 0.0f
			};
			bx::vec4MulMtx(m_spotDirectionInner_viewSpace, tmp, _viewMtx);
			m_spotDirectionInner_viewSpace[3] = m_spotDirectionInner.m_v[3];
		}


		Position              m_position;
		float				  m_position_viewSpace[4];
		LightRgbPower         m_ambientPower;
		LightRgbPower         m_diffusePower;
		LightRgbPower         m_specularPower;
		SpotDirectionInner    m_spotDirectionInner;
		float				  m_spotDirectionInner_viewSpace[4];
		AttenuationSpotOuter  m_attenuationSpotOuter;

		float	m_near;
		float	m_far;
	};

	class AgLightManager : public AgResourcePool<AgLight>
	{
	};
}