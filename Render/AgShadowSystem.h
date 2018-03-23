#pragma once

#include "AgFxSystem.h"

namespace ambergris {

	struct AgLight;
	struct AgMaterial;

	class AgShadowSystem : public AgFxSystem
	{
		struct PackDepth
		{
			enum Enum
			{
				RGBA,
				VSM,

				Count
			};
		};

		struct SmType
		{
			enum Enum
			{
				Single,
				Omni,
				Cascade,

				Count
			};
		};

		struct ShadowMapRenderTargets
		{
			enum Enum
			{
				First,
				Second,
				Third,
				Fourth,

				Count
			};
		};
	public:
		struct DepthImpl
		{
			enum Enum
			{
				InvZ,
				Linear,

				Count
			};
		};

		struct SmImpl
		{
			enum Enum
			{
				Hard,
				PCF,
				VSM,
				ESM,

				Count
			};
		};

		struct ShadowRenderState
		{
			enum Enum
			{
				Default = 0,

				ShadowMap_PackDepth,
				ShadowMap_PackDepthHoriz,
				ShadowMap_PackDepthVert,

				Custom_BlendLightTexture,
				Custom_DrawPlaneBottom,

				Count
			};
		};
	public:
		AgShadowSystem();
		virtual ~AgShadowSystem() { destroy(); }

		bool init(SmImpl::Enum	smImpl, DepthImpl::Enum depthImpl, uint16_t shadowMapSize, AgLight* light, AgMaterial* material, bool blur = false);
		void destroy();

		virtual void begin() override;
		virtual void end() override;
		virtual void setPerDrawUniforms(const AgShader* shader, const AgRenderItem* item) const override;
		virtual bool needTexture() const;
		virtual AgShader::Handle getOverrideShader() const override;
		virtual AgRenderState getOverrideStates() const override;

		void drawPackDepth(const AgRenderNode* node);
		void blurShadowMap() const;
		void prepareShadowMatrix();
	protected:
		bool _PrepareShader();
		void _UpdateSettings();
		void _SetPerFrameUniforms() const;
		void _UpdateLightTransform();
		void _ClearViews() const;
		void _CraftStencilMask() const;
		void _SwitchLightingDrawState() { m_renderStateIndex = ShadowRenderState::Default; }
	private:
		SmImpl::Enum	m_smImpl;
		DepthImpl::Enum	m_depthImpl;
		AgLight*		m_light;
		AgMaterial*		m_material;

		//runtime
		uint16_t		m_currentShadowMapSize;
		uint8_t			m_target_num;
		ShadowRenderState::Enum	m_renderStateIndex;
		AgShader::Handle	m_currentPackDepthShader;
		AgShader::Handle	m_currentLightingShader;
		float			m_mtxShadow[16];
		mutable float	m_lightMtx[16];
		float			m_lightView[ShadowMapRenderTargets::Count][16];
		float			m_lightProj[ShadowMapRenderTargets::Count][16];
		float			m_shadowMapMtx[ShadowMapRenderTargets::Count][16];

		bgfx::VertexDecl m_posDecl;

		bgfx::UniformHandle		m_shadowMap[ShadowMapRenderTargets::Count];
		bgfx::FrameBufferHandle m_rtShadowMap[ShadowMapRenderTargets::Count];
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