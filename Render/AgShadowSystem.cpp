#include "AgShadowSystem.h"

#include "AgRenderPass.h"
#include "Resource/AgDirectionalLight.h"
#include "Resource/AgPointLight.h"
#include "Resource/AgSpotLight.h"
#include "Resource/AgMaterial.h"
#include "Resource/AgRenderResourceManager.h"
#include "Resource/AgShaderUtils.h"

namespace ambergris {

	struct RenderState
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

		uint64_t m_state;
		uint32_t m_blendFactorRgba;
		uint32_t m_fstencil;
		uint32_t m_bstencil;
	};

	static RenderState s_renderStates[RenderState::Count] =
	{
		{ // Default
			0
			| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA
		, UINT32_MAX
		, BGFX_STENCIL_NONE
		, BGFX_STENCIL_NONE
		},
		{ // ShadowMap_PackDepth
			0
			| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA
		, UINT32_MAX
		, BGFX_STENCIL_NONE
		, BGFX_STENCIL_NONE
		},
		{ // ShadowMap_PackDepthHoriz
			0
			| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA
		, UINT32_MAX
		, BGFX_STENCIL_TEST_EQUAL
		| BGFX_STENCIL_FUNC_REF(1)
		| BGFX_STENCIL_FUNC_RMASK(0xff)
		| BGFX_STENCIL_OP_FAIL_S_KEEP
		| BGFX_STENCIL_OP_FAIL_Z_KEEP
		| BGFX_STENCIL_OP_PASS_Z_KEEP
		, BGFX_STENCIL_NONE
		},
		{ // ShadowMap_PackDepthVert
			0
			| BGFX_STATE_WRITE_RGB
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA
		, UINT32_MAX
		, BGFX_STENCIL_TEST_EQUAL
		| BGFX_STENCIL_FUNC_REF(0)
		| BGFX_STENCIL_FUNC_RMASK(0xff)
		| BGFX_STENCIL_OP_FAIL_S_KEEP
		| BGFX_STENCIL_OP_FAIL_Z_KEEP
		| BGFX_STENCIL_OP_PASS_Z_KEEP
		, BGFX_STENCIL_NONE
		},
		{ // Custom_BlendLightTexture
			BGFX_STATE_WRITE_RGB
			| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_COLOR, BGFX_STATE_BLEND_INV_SRC_COLOR)
		| BGFX_STATE_CULL_CCW
		| BGFX_STATE_MSAA
		, UINT32_MAX
		, BGFX_STENCIL_NONE
		, BGFX_STENCIL_NONE
		},
		{ // Custom_DrawPlaneBottom
			BGFX_STATE_WRITE_RGB
			| BGFX_STATE_CULL_CW
		| BGFX_STATE_MSAA
		, UINT32_MAX
		, BGFX_STENCIL_NONE
		, BGFX_STENCIL_NONE
		},
	};

	AgShadowSystem::AgShadowSystem() : 
		AgFxSystem(),
		m_smImpl(E_SM_IMPL_Hard),
		m_depthImpl(E_DEPTH_InvZ),
		m_currentShadowMapSize(1024),
		m_target_num(1),
		m_flipV(false), m_texelHalf(0.0f)
	{}

	bool AgShadowSystem::_PrepareShader()
	{
		{
			AgShader* invz_hard_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_INVZ_HARD);
			if (!invz_hard_shader)
				return false;

			if (!invz_hard_shader->m_prepared)
			{
				invz_hard_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_color_lighting", "fs_shadowmaps_color_lighting_hard");
				if (!bgfx::isValid(invz_hard_shader->m_program))
					return false;

				invz_hard_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_params0", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_params1", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_params2", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_color", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[4].uniform_handle = bgfx::createUniform("u_smSamplingParams", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[5].uniform_handle = bgfx::createUniform("u_csmFarDistances", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[6].uniform_handle = bgfx::createUniform("u_lightMtx", bgfx::UniformType::Mat4);

				invz_hard_shader->m_uniforms[7].uniform_handle = bgfx::createUniform("u_lightPosition", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[8].uniform_handle = bgfx::createUniform("u_lightAmbientPower", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[9].uniform_handle = bgfx::createUniform("u_lightDiffusePower", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[10].uniform_handle = bgfx::createUniform("u_lightSpecularPower", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[11].uniform_handle = bgfx::createUniform("u_lightSpotDirectionInner", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[12].uniform_handle = bgfx::createUniform("u_lightAttenuationSpotOuter", bgfx::UniformType::Vec4);

				invz_hard_shader->m_uniforms[13].uniform_handle = bgfx::createUniform("u_materialKa", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[14].uniform_handle = bgfx::createUniform("u_materialKd", bgfx::UniformType::Vec4);
				invz_hard_shader->m_uniforms[15].uniform_handle = bgfx::createUniform("u_materialKs", bgfx::UniformType::Vec4);

				invz_hard_shader->m_uniforms[16].uniform_handle = bgfx::createUniform("u_shadowMapMtx0", bgfx::UniformType::Mat4);
				{
					invz_hard_shader->m_uniforms[17].uniform_handle = bgfx::createUniform("u_shadowMapMtx1", bgfx::UniformType::Mat4);
					invz_hard_shader->m_uniforms[18].uniform_handle = bgfx::createUniform("u_shadowMapMtx2", bgfx::UniformType::Mat4);
					invz_hard_shader->m_uniforms[19].uniform_handle = bgfx::createUniform("u_shadowMapMtx3", bgfx::UniformType::Mat4);
				}
				
				{
					invz_hard_shader->m_uniforms[20].uniform_handle = bgfx::createUniform("u_tetraNormalGreen", bgfx::UniformType::Vec4);
					invz_hard_shader->m_uniforms[21].uniform_handle = bgfx::createUniform("u_tetraNormalYellow", bgfx::UniformType::Vec4);
					invz_hard_shader->m_uniforms[22].uniform_handle = bgfx::createUniform("u_tetraNormalBlue", bgfx::UniformType::Vec4);
					invz_hard_shader->m_uniforms[23].uniform_handle = bgfx::createUniform("u_tetraNormalRed", bgfx::UniformType::Vec4);
				}

				// Textures
				invz_hard_shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
				invz_hard_shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;

				invz_hard_shader->m_prepared = true;
			}
		}
		return true;
	}

	bool AgShadowSystem::init(ShadowMapImpl	smImpl, DepthImpl depthImpl, uint16_t shadowMapSize, AgLight* light, AgMaterial* material, bool blur /*= false*/)
	{
		/*if (!light)
			return false;*/

		if (!_PrepareShader())
		{
			printf("Failed to load shadow shaders.\n");
			return false;
		}

		m_smImpl = smImpl;
		m_depthImpl = depthImpl;
		m_currentShadowMapSize = shadowMapSize;
		m_light = light;
		m_material = material;

		// Setup root path for binary shaders. Shader binaries are different
		// for each renderer.
		switch (bgfx::getRendererType())
		{
		case bgfx::RendererType::Direct3D9:
			m_texelHalf = 0.5f;
			break;

		case bgfx::RendererType::OpenGL:
		case bgfx::RendererType::OpenGLES:
			m_flipV = true;
			break;

		default:
			break;
		}

		float currentShadowMapSizef = float(m_currentShadowMapSize);
		m_uniforms.m_shadowMapTexelSize = 1.0f / currentShadowMapSizef;

		destroy();
		
		if (typeid(AgDirectionalLight) == typeid(*m_light))
		{
			m_target_num = 4;
		}
		else
		{
			m_target_num = 1;
		}

		for (uint8_t ii = 0; ii < m_target_num; ++ii)
		{
			bgfx::TextureHandle fbtextures[] =
			{
				bgfx::createTexture2D(m_currentShadowMapSize, m_currentShadowMapSize, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT),
				bgfx::createTexture2D(m_currentShadowMapSize, m_currentShadowMapSize, false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT),
			};
			m_rtShadowMap[ii] = bgfx::createFrameBuffer(BX_COUNTOF(fbtextures), fbtextures, true);
		}
		if(blur && (E_SM_IMPL_VSM == m_smImpl || E_SM_IMPL_ESM == m_smImpl))
			m_rtBlur = bgfx::createFrameBuffer(m_currentShadowMapSize, m_currentShadowMapSize, bgfx::TextureFormat::BGRA8);

		return true;
	}

	void AgShadowSystem::destroy()
	{
		// clear buffer
		for (uint8_t ii = 0; ii < E_TARGET_Count; ++ii)
		{
			if (bgfx::isValid(m_rtShadowMap[ii]))
			{
				bgfx::destroy(m_rtShadowMap[ii]);
			}
			m_rtShadowMap[ii].idx = bgfx::kInvalidHandle;
		}
		if (bgfx::isValid(m_rtBlur))
		{
			bgfx::destroy(m_rtBlur);
		}
		m_rtBlur.idx = bgfx::kInvalidHandle;
	}

	/*virtual*/
	void AgShadowSystem::setPerFrameUniforms() const 
	{
		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(getOverrideShader());
		if (!shader)
			return;

		bgfx::setUniform(shader->m_uniforms[1].uniform_handle, m_uniforms.m_params1);
		bgfx::setUniform(shader->m_uniforms[2].uniform_handle, m_uniforms.m_params2);
		bgfx::setUniform(shader->m_uniforms[4].uniform_handle, m_uniforms.m_paramsBlur);
		bgfx::setUniform(shader->m_uniforms[5].uniform_handle, m_uniforms.m_csmFarDistances);

		if (m_material)
		{
			bgfx::setUniform(shader->m_uniforms[13].uniform_handle, m_material->m_ka.m_v);
			bgfx::setUniform(shader->m_uniforms[14].uniform_handle, m_material->m_kd.m_v);
			bgfx::setUniform(shader->m_uniforms[15].uniform_handle, m_material->m_ks.m_v);
		}
		
		if (m_light)
		{
			bgfx::setUniform(shader->m_uniforms[7].uniform_handle, m_light->m_position_viewSpace);
			bgfx::setUniform(shader->m_uniforms[8].uniform_handle, m_light->m_ambientPower.m_v);
			bgfx::setUniform(shader->m_uniforms[9].uniform_handle, m_light->m_diffusePower.m_v);
			bgfx::setUniform(shader->m_uniforms[10].uniform_handle, m_light->m_specularPower.m_v);
			bgfx::setUniform(shader->m_uniforms[11].uniform_handle, m_light->m_spotDirectionInner_viewSpace);
			bgfx::setUniform(shader->m_uniforms[12].uniform_handle, m_light->m_attenuationSpotOuter.m_v);
		}

		if (false)
		{
			//TODO: Call this once at initialization.
			bgfx::setUniform(shader->m_uniforms[20].uniform_handle, m_uniforms.m_tetraNormalGreen);
			bgfx::setUniform(shader->m_uniforms[21].uniform_handle, m_uniforms.m_tetraNormalYellow);
			bgfx::setUniform(shader->m_uniforms[22].uniform_handle, m_uniforms.m_tetraNormalBlue);
			bgfx::setUniform(shader->m_uniforms[23].uniform_handle, m_uniforms.m_tetraNormalRed);
		}
	}

	/*virtual*/
	void AgShadowSystem::setPerDrawUniforms(const AgShader* shader, void* data) const 
	{
		bgfx::setUniform(shader->m_uniforms[16].uniform_handle, m_shadowMapMtx[E_TARGET_First]);
		{
			bgfx::setUniform(shader->m_uniforms[17].uniform_handle, m_shadowMapMtx[E_TARGET_Second]);
			bgfx::setUniform(shader->m_uniforms[18].uniform_handle, m_shadowMapMtx[E_TARGET_Third]);
			bgfx::setUniform(shader->m_uniforms[19].uniform_handle, m_shadowMapMtx[E_TARGET_Fourth]);
		}
		
		bgfx::setUniform(shader->m_uniforms[0].uniform_handle, m_uniforms.m_params0);
		bgfx::setUniform(shader->m_uniforms[6].uniform_handle, m_lightMtx);
		bgfx::setUniform(shader->m_uniforms[3].uniform_handle, m_uniforms.m_color);
	}

	/*virtual*/
	uint64_t AgShadowSystem::getOverrideStates() const
	{
		return s_renderStates[0].m_state;
	}

	void AgShadowSystem::_UpdateUniforms()
	{
		if (!m_light)
			return;

		m_uniforms.m_showSmCoverage = float(true);// show coverage
		float currentShadowMapSizef = float(m_currentShadowMapSize);
		m_uniforms.m_shadowMapTexelSize = 1.0f / currentShadowMapSizef;

		// Update uniforms.
		if (typeid(AgPointLight) == typeid(*m_light))
		{
			m_light->m_attenuationSpotOuter.m_outer = 91.0f; //above 90.0f means point light
		}
		else if (typeid(AgDirectionalLight) == typeid(*m_light))
		{
			m_light->m_attenuationSpotOuter.m_outer = 91.0f; //above 90.0f means point light
		}
		else //spot light
		{
			if (E_DEPTH_InvZ == m_depthImpl)
			{
				if (E_SM_IMPL_Hard == m_smImpl)
				{
					m_uniforms.m_shadowMapBias = 0.0035f;
					m_uniforms.m_shadowMapOffset = 0.0012f;
					m_uniforms.m_shadowMapParam0 = 0.7f;
					m_uniforms.m_shadowMapParam1 = 500.0f;
					m_uniforms.m_depthValuePow = 10.0f;
					m_uniforms.m_XNum = 2.0f;
					m_uniforms.m_YNum = 2.0f;
					m_uniforms.m_XOffset = 1.0f;
					m_uniforms.m_YOffset = 1.0f;
				}
				else
				{

				}
			}
			else
			{

			}
		}
	}

	void AgShadowSystem::_UpdateLightTransform()
	{
		if (!m_light)
			return;

		float lightView[E_TARGET_Count][16];
		float lightProj[E_TARGET_Count][16];

		m_light->prepareShadowMap((float**)lightView, (float**)lightProj, E_DEPTH_Linear == m_depthImpl, m_currentShadowMapSize);

		AgDirectionalLight* directionalLight = dynamic_cast<AgDirectionalLight*>(m_light);
		if (directionalLight)
		{
			// Update uniforms.
			for (uint8_t ii = 0, ff = 1; ii < directionalLight->m_numSplits; ++ii, ff += 2)
			{
				// This lags for 1 frame, but it's not a problem.
				m_uniforms.m_csmFarDistances[ii] = directionalLight->m_splitSlices[ff];
			}
		}
		
		// Set render target
		float screenProj[16];
		float screenView[16];
		bx::mtxIdentity(screenView);

		bx::mtxOrtho(
			screenProj
			, 0.0f
			, 1.0f
			, 1.0f
			, 0.0f
			, 0.0f
			, 100.0f
			, 0.0f
			, bgfx::getCaps()->homogeneousDepth
		);

		// Reset render targets.
		const bgfx::FrameBufferHandle invalidRt = BGFX_INVALID_HANDLE;
		for (uint8_t ii = AgRenderPass::E_VIEW_SHADOWMAP_0_ID; ii < AgRenderPass::E_VIEW_HBLUR_3_ID + 1; ++ii)
		{
			bgfx::setViewFrameBuffer(ii, invalidRt);
		}

		if (typeid(AgSpotLight) == typeid(*m_light))
		{
			/**
			* RENDERVIEW_SHADOWMAP_0_ID - Clear shadow map. (used as convenience, otherwise render_pass_1 could be cleared)
			* RENDERVIEW_SHADOWMAP_1_ID - Craft shadow map.
			* RENDERVIEW_VBLUR_0_ID - Vertical blur.
			* RENDERVIEW_HBLUR_0_ID - Horizontal blur.
			* RENDERVIEW_DRAWSCENE_0_ID - Draw scene.
			* RENDERVIEW_DRAWSCENE_1_ID - Draw floor bottom.
			* RENDERVIEW_DRAWDEPTH_0_ID - Draw depth buffer.
			*/
			bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
			bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_0_ID, screenView, screenProj);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, lightView[0], lightProj[AgLight::E_PROJ_Horizontal]);

			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_0_ID, m_rtShadowMap[0]);
			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, m_rtShadowMap[0]);

			if (bgfx::isValid(m_rtBlur))
			{
				bgfx::setViewRect(AgRenderPass::E_VIEW_VBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_HBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

				bgfx::setViewTransform(AgRenderPass::E_VIEW_VBLUR_0_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_HBLUR_0_ID, screenView, screenProj);

				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_VBLUR_0_ID, m_rtBlur);
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_HBLUR_0_ID, m_rtShadowMap[0]);
			}
		}
		else if (typeid(AgPointLight) == typeid(*m_light))
		{
			/**
			* RENDERVIEW_SHADOWMAP_0_ID - Clear entire shadow map.
			* RENDERVIEW_SHADOWMAP_1_ID - Craft green tetrahedron shadow face.
			* RENDERVIEW_SHADOWMAP_2_ID - Craft yellow tetrahedron shadow face.
			* RENDERVIEW_SHADOWMAP_3_ID - Craft blue tetrahedron shadow face.
			* RENDERVIEW_SHADOWMAP_4_ID - Craft red tetrahedron shadow face.
			* RENDERVIEW_VBLUR_0_ID - Vertical blur.
			* RENDERVIEW_HBLUR_0_ID - Horizontal blur.
			* RENDERVIEW_DRAWSCENE_0_ID - Draw scene.
			* RENDERVIEW_DRAWSCENE_1_ID - Draw floor bottom.
			* RENDERVIEW_DRAWDEPTH_0_ID - Draw depth buffer.
			*/
			AgPointLight* pointLight = dynamic_cast<AgPointLight*>(m_light);

			bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_0_ID, screenView, screenProj);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, lightView[AgPointLight::E_TETRA_Green], lightProj[AgLight::E_PROJ_Horizontal]);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, lightView[AgPointLight::E_TETRA_Yellow], lightProj[AgLight::E_PROJ_Horizontal]);

			if (pointLight->m_stencilPack)
			{
				const uint16_t f = m_currentShadowMapSize;   //full size
				const uint16_t h = m_currentShadowMapSize / 2; //half size
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, 0, 0, f, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, 0, h, f, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, 0, 0, h, f);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, h, 0, h, f);

				bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, lightView[AgPointLight::E_TETRA_Blue], lightProj[AgLight::E_PROJ_Vertical]);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, lightView[AgPointLight::E_TETRA_Red], lightProj[AgLight::E_PROJ_Vertical]);
			}
			else
			{
				const uint16_t h = m_currentShadowMapSize / 2; //half size
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, 0, 0, h, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, h, 0, h, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, 0, h, h, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, h, h, h, h);

				bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, lightView[AgPointLight::E_TETRA_Blue], lightProj[AgLight::E_PROJ_Horizontal]);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, lightView[AgPointLight::E_TETRA_Red], lightProj[AgLight::E_PROJ_Horizontal]);
			}


			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_0_ID, m_rtShadowMap[0]);
			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, m_rtShadowMap[0]);
			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, m_rtShadowMap[0]);
			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, m_rtShadowMap[0]);
			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, m_rtShadowMap[0]);

			if (bgfx::isValid(m_rtBlur))
			{
				bgfx::setViewRect(AgRenderPass::E_VIEW_VBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_HBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

				bgfx::setViewTransform(AgRenderPass::E_VIEW_VBLUR_0_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_HBLUR_0_ID, screenView, screenProj);

				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_VBLUR_0_ID, m_rtBlur);
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_HBLUR_0_ID, m_rtShadowMap[0]);
			}
		}
		else if (typeid(AgDirectionalLight) == typeid(*m_light))
		{
			/**
			* RENDERVIEW_SHADOWMAP_1_ID - Craft shadow map for first  split.
			* RENDERVIEW_SHADOWMAP_2_ID - Craft shadow map for second split.
			* RENDERVIEW_SHADOWMAP_3_ID - Craft shadow map for third  split.
			* RENDERVIEW_SHADOWMAP_4_ID - Craft shadow map for fourth split.
			* RENDERVIEW_VBLUR_0_ID - Vertical   blur for first  split.
			* RENDERVIEW_HBLUR_0_ID - Horizontal blur for first  split.
			* RENDERVIEW_VBLUR_1_ID - Vertical   blur for second split.
			* RENDERVIEW_HBLUR_1_ID - Horizontal blur for second split.
			* RENDERVIEW_VBLUR_2_ID - Vertical   blur for third  split.
			* RENDERVIEW_HBLUR_2_ID - Horizontal blur for third  split.
			* RENDERVIEW_VBLUR_3_ID - Vertical   blur for fourth split.
			* RENDERVIEW_HBLUR_3_ID - Horizontal blur for fourth split.
			* RENDERVIEW_DRAWSCENE_0_ID - Draw scene.
			* RENDERVIEW_DRAWSCENE_1_ID - Draw floor bottom.
			* RENDERVIEW_DRAWDEPTH_0_ID - Draw depth buffer for first  split.
			* RENDERVIEW_DRAWDEPTH_1_ID - Draw depth buffer for second split.
			* RENDERVIEW_DRAWDEPTH_2_ID - Draw depth buffer for third  split.
			* RENDERVIEW_DRAWDEPTH_3_ID - Draw depth buffer for fourth split.
			*/

			bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
			bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
			bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
			bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, lightView[0], lightProj[0]);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, lightView[0], lightProj[1]);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, lightView[0], lightProj[2]);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, lightView[0], lightProj[3]);

			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, m_rtShadowMap[0]);
			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, m_rtShadowMap[1]);
			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, m_rtShadowMap[2]);
			bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, m_rtShadowMap[3]);

			if (bgfx::isValid(m_rtBlur))
			{
				bgfx::setViewRect(AgRenderPass::E_VIEW_VBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_HBLUR_0_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_VBLUR_1_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_HBLUR_1_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_VBLUR_2_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_HBLUR_2_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_VBLUR_3_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);
				bgfx::setViewRect(AgRenderPass::E_VIEW_HBLUR_3_ID, 0, 0, m_currentShadowMapSize, m_currentShadowMapSize);

				bgfx::setViewTransform(AgRenderPass::E_VIEW_VBLUR_0_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_HBLUR_0_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_VBLUR_1_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_HBLUR_1_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_VBLUR_2_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_HBLUR_2_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_VBLUR_3_ID, screenView, screenProj);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_HBLUR_3_ID, screenView, screenProj);

				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_VBLUR_0_ID, m_rtBlur);         //vblur
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_HBLUR_0_ID, m_rtShadowMap[0]); //hblur
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_VBLUR_1_ID, m_rtBlur);         //vblur
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_HBLUR_1_ID, m_rtShadowMap[1]); //hblur
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_VBLUR_2_ID, m_rtBlur);         //vblur
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_HBLUR_2_ID, m_rtShadowMap[2]); //hblur
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_VBLUR_3_ID, m_rtBlur);         //vblur
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_HBLUR_3_ID, m_rtShadowMap[3]); //hblur
			}
		}
	}
}