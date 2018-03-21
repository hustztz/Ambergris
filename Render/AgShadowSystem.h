#pragma once

#include "AgFxSystem.h"

namespace ambergris {

	struct AgLight;
	struct AgMaterial;

	class AgShadowSystem : public AgFxSystem
	{
		enum DepthImpl
		{
			E_DEPTH_InvZ,
			E_DEPTH_Linear,

			E_DEPTH_Count
		};

		enum PackDepth
		{
			E_PACK_RGBA,
			E_PACK_VSM,

			E_PACK_Count
		};

		enum ShadowMapImpl
		{
			E_SM_IMPL_Hard,
			E_SM_IMPL_PCF,
			E_SM_IMPL_VSM,
			E_SM_IMPL_ESM,

			E_SM_IMPL_Count
		};

		enum ShadowMapType
		{
			E_SM_TYPE_Single,
			E_SM_TYPE_Omni,
			E_SM_TYPE_Cascade,

			E_SM_TYPE_Count
		};

		enum ShadowMapRenderTargets
		{
			E_TARGET_First,
			E_TARGET_Second,
			E_TARGET_Third,
			E_TARGET_Fourth,

			E_TARGET_Count
		};
	public:
		AgShadowSystem();
		virtual ~AgShadowSystem() { destroy(); }

		bool init(ShadowMapImpl	smImpl, DepthImpl depthImpl, uint16_t shadowMapSize, AgLight* light, AgMaterial* material, bool blur = false);
		void destroy();

		virtual void setPerFrameUniforms() const override;
		virtual void setPerDrawUniforms(const AgShader* shader, void* data) const override;
		virtual bool needTexture() const { return false; }
		virtual AgShader::Handle getOverrideShader() const override { return AgShader::E_SIMPLE_SHADER; }
		virtual uint64_t getOverrideStates() const override;

	protected:
		bool _PrepareShader();
		void _UpdateUniforms();
		void _UpdateLightTransform();
	private:
		ShadowMapImpl	m_smImpl;
		DepthImpl		m_depthImpl;
		uint16_t		m_currentShadowMapSize;
		AgLight*		m_light;
		AgMaterial*		m_material;

		uint8_t			m_target_num;
		float			m_lightMtx[16];
		float			m_shadowMapMtx[E_TARGET_Count][16];

		bgfx::FrameBufferHandle m_rtShadowMap[E_TARGET_Count];
		bgfx::FrameBufferHandle m_rtBlur;
		bool m_flipV;
		float m_texelHalf;

		// Uniforms
		struct Uniforms
		{
			union
			{
				struct
				{
					float m_ambientPass;
					float m_lightingPass;
					float m_unused00;
					float m_unused01;
				};

				float m_params0[4];
			};

			union
			{
				struct
				{
					float m_shadowMapBias;
					float m_shadowMapOffset;
					float m_shadowMapParam0;
					float m_shadowMapParam1;
				};

				float m_params1[4];
			};

			union
			{
				struct
				{
					float m_depthValuePow;
					float m_showSmCoverage;
					float m_shadowMapTexelSize;
					float m_unused23;
				};

				float m_params2[4];
			};

			union
			{
				struct
				{
					float m_XNum;
					float m_YNum;
					float m_XOffset;
					float m_YOffset;
				};

				float m_paramsBlur[4];
			};

			float m_color[4];

			float m_tetraNormalGreen[3];
			float m_tetraNormalYellow[3];
			float m_tetraNormalBlue[3];
			float m_tetraNormalRed[3];
			float m_csmFarDistances[4];

			Uniforms()
			{
				m_ambientPass = 1.0f;
				m_lightingPass = 1.0f;

				m_shadowMapBias = 0.003f;
				m_shadowMapOffset = 0.0f;
				m_shadowMapParam0 = 0.5;
				m_shadowMapParam1 = 1.0;
				m_depthValuePow = 1.0f;
				m_showSmCoverage = 1.0f;
				m_shadowMapTexelSize = 1.0f / 512.0f;

				m_csmFarDistances[0] = 30.0f;
				m_csmFarDistances[1] = 90.0f;
				m_csmFarDistances[2] = 180.0f;
				m_csmFarDistances[3] = 1000.0f;

				m_tetraNormalGreen[0] = 0.0f;
				m_tetraNormalGreen[1] = -0.57735026f;
				m_tetraNormalGreen[2] = 0.81649661f;

				m_tetraNormalYellow[0] = 0.0f;
				m_tetraNormalYellow[1] = -0.57735026f;
				m_tetraNormalYellow[2] = -0.81649661f;

				m_tetraNormalBlue[0] = -0.81649661f;
				m_tetraNormalBlue[1] = 0.57735026f;
				m_tetraNormalBlue[2] = 0.0f;

				m_tetraNormalRed[0] = 0.81649661f;
				m_tetraNormalRed[1] = 0.57735026f;
				m_tetraNormalRed[2] = 0.0f;

				m_XNum = 2.0f;
				m_YNum = 2.0f;
				m_XOffset = 10.0f / 512.0f;
				m_YOffset = 10.0f / 512.0f;

				m_color[0] = m_color[1] = m_color[2] = m_color[3] = 1.0f;
			}

		} m_uniforms;
	};
}