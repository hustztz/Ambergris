#include "AgShadowSystem.h"
#include "AgRenderNode.h"
#include "AgRenderItem.h"
#include "Resource/AgRenderPass.h"
#include "Resource/AgDirectionalLight.h"
#include "Resource/AgPointLight.h"
#include "Resource/AgSpotLight.h"
#include "Resource/AgMaterial.h"
#include "Resource/AgRenderResourceManager.h"
#include "Resource/AgShaderUtils.h"

namespace ambergris {

	static AgRenderState s_renderStates[AgShadowSystem::ShadowRenderState::Count] =
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
		m_smImpl(SmImpl::Hard),
		m_depthImpl(DepthImpl::InvZ),
		m_currentShadowMapSize(1024),
		m_target_num(1),
		m_renderStateIndex(ShadowRenderState::Default),
		m_currentPackDepthShader(AgShader::kInvalidHandle),
		m_currentLightingShader(AgShader::kInvalidHandle),
		m_flipV(false), m_texelHalf(0.0f)
	{
		for (uint8_t ii = 0; ii < ShadowMapRenderTargets::Count; ++ii)
		{
			m_rtShadowMap[ii].idx = bgfx::kInvalidHandle;
			m_shadowMap[ii].idx = bgfx::kInvalidHandle;
		}
		m_rtBlur.idx = bgfx::kInvalidHandle;

		m_posDecl.begin();
		m_posDecl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
		m_posDecl.end();
	}

	bool AgShadowSystem::_PrepareShader()
	{
		{
			AgShader* black_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_BLACK);
			if (!black_shader)
				return false;

			if (AgShader::kInvalidHandle == black_shader->m_handle)
			{
				black_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_color", "fs_shadowmaps_color_black");
				if (!bgfx::isValid(black_shader->m_program))
					return false;

				black_shader->m_handle = AgShader::E_SHADOW_BLACK;
			}
		}
		{
			AgShader* vBlur_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_VBLUR);
			if (!vBlur_shader)
				return false;

			if (AgShader::kInvalidHandle == vBlur_shader->m_handle)
			{
				vBlur_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_vblur", "fs_shadowmaps_vblur");
				if (!bgfx::isValid(vBlur_shader->m_program))
					return false;

				vBlur_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_smSamplingParams", bgfx::UniformType::Vec4);
				vBlur_shader->m_handle = AgShader::E_SHADOW_VBLUR;
			}
		}
		{
			AgShader* hBlur_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_HBLUR);
			if (!hBlur_shader)
				return false;

			if (AgShader::kInvalidHandle == hBlur_shader->m_handle)
			{
				hBlur_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_hblur", "fs_shadowmaps_hblur");
				if (!bgfx::isValid(hBlur_shader->m_program))
					return false;

				hBlur_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_smSamplingParams", bgfx::UniformType::Vec4);
				hBlur_shader->m_handle = AgShader::E_SHADOW_HBLUR;
			}
		}
		{
			AgShader* vBlur_vsm_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_VBLUR_VSM);
			if (!vBlur_vsm_shader)
				return false;

			if (AgShader::kInvalidHandle == vBlur_vsm_shader->m_handle)
			{
				vBlur_vsm_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_vblur", "fs_shadowmaps_vblur_vsm");
				if (!bgfx::isValid(vBlur_vsm_shader->m_program))
					return false;

				vBlur_vsm_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_smSamplingParams", bgfx::UniformType::Vec4);
				vBlur_vsm_shader->m_handle = AgShader::E_SHADOW_VBLUR_VSM;
			}
		}
		{
			AgShader* hBlur_vsm_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_HBLUR_VSM);
			if (!hBlur_vsm_shader)
				return false;

			if (AgShader::kInvalidHandle == hBlur_vsm_shader->m_handle)
			{
				hBlur_vsm_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_hblur", "fs_shadowmaps_hblur_vsm");
				if (!bgfx::isValid(hBlur_vsm_shader->m_program))
					return false;

				hBlur_vsm_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_smSamplingParams", bgfx::UniformType::Vec4);
				hBlur_vsm_shader->m_handle = AgShader::E_SHADOW_HBLUR_VSM;
			}
		}
		{
			AgShader* packDepth_invz_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_PACKDEPTH_INVZ);
			if (!packDepth_invz_shader)
				return false;

			if (AgShader::kInvalidHandle == packDepth_invz_shader->m_handle)
			{
				packDepth_invz_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_packdepth", "fs_shadowmaps_packdepth");
				if (!bgfx::isValid(packDepth_invz_shader->m_program))
					return false;

				packDepth_invz_shader->m_handle = AgShader::E_SHADOW_PACKDEPTH_INVZ;
			}
		}
		{
			AgShader* packDepth_invz_vsm_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_PACKDEPTH_INVZ_VSM);
			if (!packDepth_invz_vsm_shader)
				return false;

			if (AgShader::kInvalidHandle == packDepth_invz_vsm_shader->m_handle)
			{
				packDepth_invz_vsm_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_packdepth", "fs_shadowmaps_packdepth_vsm");
				if (!bgfx::isValid(packDepth_invz_vsm_shader->m_program))
					return false;

				packDepth_invz_vsm_shader->m_handle = AgShader::E_SHADOW_PACKDEPTH_INVZ_VSM;
			}
		}
		{
			AgShader* packDepth_linear_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_PACKDEPTH_LINEAR);
			if (!packDepth_linear_shader)
				return false;

			if (AgShader::kInvalidHandle == packDepth_linear_shader->m_handle)
			{
				packDepth_linear_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_packdepth_linear", "fs_shadowmaps_packdepth_linear");
				if (!bgfx::isValid(packDepth_linear_shader->m_program))
					return false;

				packDepth_linear_shader->m_handle = AgShader::E_SHADOW_PACKDEPTH_LINEAR;
			}
		}
		{
			AgShader* packDepth_linear_vsm_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_PACKDEPTH_LINEAR_VSM);
			if (!packDepth_linear_vsm_shader)
				return false;

			if (AgShader::kInvalidHandle == packDepth_linear_vsm_shader->m_handle)
			{
				packDepth_linear_vsm_shader->m_program = shaderUtils::loadProgram("vs_shadowmaps_packdepth_linear", "fs_shadowmaps_packdepth_vsm_linear");
				if (!bgfx::isValid(packDepth_linear_vsm_shader->m_program))
					return false;

				packDepth_linear_vsm_shader->m_handle = AgShader::E_SHADOW_PACKDEPTH_LINEAR_VSM;
			}
		}
		{
			AgShader* invz_hard_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_INVZ_HARD);
			if (!invz_hard_shader)
				return false;

			if (AgShader::kInvalidHandle == invz_hard_shader->m_handle)
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

				invz_hard_shader->m_handle = AgShader::E_SHADOW_INVZ_HARD;
			}

			m_shadowMap[0] = bgfx::createUniform("s_shadowMap0", bgfx::UniformType::Int1);
			m_shadowMap[1] = bgfx::createUniform("s_shadowMap1", bgfx::UniformType::Int1);
			m_shadowMap[2] = bgfx::createUniform("s_shadowMap2", bgfx::UniformType::Int1);
			m_shadowMap[3] = bgfx::createUniform("s_shadowMap3", bgfx::UniformType::Int1);

		}
		return true;
	}

	bool AgShadowSystem::init(SmImpl::Enum	smImpl, DepthImpl::Enum depthImpl, uint16_t shadowMapSize, AgLight* light, AgMaterial* material, bool blur /*= false*/)
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

		_UpdateSettings();

		float currentShadowMapSizef = float(m_currentShadowMapSize);
		m_uniforms.m_shadowMapTexelSize = 1.0f / currentShadowMapSizef;

		destroy();

		AgDirectionalLight* directionalLight = dynamic_cast<AgDirectionalLight*>(m_light);
		if (directionalLight)
		{
			m_target_num = uint8_t(directionalLight->m_numSplits);
		}
		else if (typeid(AgPointLight) == typeid(*m_light))
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
		if(blur && (SmImpl::VSM == m_smImpl || SmImpl::ESM == m_smImpl))
			m_rtBlur = bgfx::createFrameBuffer(m_currentShadowMapSize, m_currentShadowMapSize, bgfx::TextureFormat::BGRA8);

		return true;
	}

	void AgShadowSystem::destroy()
	{
		// clear buffer
		for (uint8_t ii = 0; ii < ShadowMapRenderTargets::Count; ++ii)
		{
			if (bgfx::isValid(m_rtShadowMap[ii]))
			{
				bgfx::destroy(m_rtShadowMap[ii]);
			}
			m_rtShadowMap[ii].idx = bgfx::kInvalidHandle;

			if (bgfx::isValid(m_shadowMap[ii]))
			{
				bgfx::destroy(m_shadowMap[ii]);
			}
			m_shadowMap[ii].idx = bgfx::kInvalidHandle;
		}
		if (bgfx::isValid(m_rtBlur))
		{
			bgfx::destroy(m_rtBlur);
		}
		m_rtBlur.idx = bgfx::kInvalidHandle;
	}

	void AgShadowSystem::_SetPerFrameUniforms() const 
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
	void AgShadowSystem::setPerDrawUniforms(const AgShader* shader, const AgRenderItem* item) const
	{
		if (!shader || !item)
			return;

		if (ShadowRenderState::ShadowMap_PackDepth == m_renderStateIndex ||
			ShadowRenderState::ShadowMap_PackDepthHoriz == m_renderStateIndex ||
			ShadowRenderState::ShadowMap_PackDepthVert == m_renderStateIndex)
			return;

		if (m_light && typeid(AgDirectionalLight) == typeid(*m_light))
		{
			const AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(item->m_transform);
			if (transform)
			{
				float transformData[16];
				transform->getFloatTransform(transformData);
				bx::mtxMul(m_lightMtx, transformData, m_mtxShadow); //not needed for directional light
			}
		}

		for (uint8_t ii = 0; ii < m_target_num; ++ii)
		{
			bgfx::setUniform(shader->m_uniforms[16 + ii].uniform_handle, m_shadowMapMtx[ShadowMapRenderTargets::First + ii]);
		}
		
		bgfx::setUniform(shader->m_uniforms[0].uniform_handle, m_uniforms.m_params0);
		bgfx::setUniform(shader->m_uniforms[6].uniform_handle, m_lightMtx);
		bgfx::setUniform(shader->m_uniforms[3].uniform_handle, m_uniforms.m_color);
	}

	void AgShadowSystem::_UpdateSettings()
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
			if (DepthImpl::InvZ == m_depthImpl)
			{
				if (SmImpl::Hard == m_smImpl)
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

					const AgShader* packDepthShader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_PACKDEPTH_INVZ);
					m_currentPackDepthShader = packDepthShader ? packDepthShader->m_handle : AgShader::kInvalidHandle;
					const AgShader* lightingShader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_INVZ_HARD);
					m_currentLightingShader = lightingShader ? lightingShader->m_handle : AgShader::kInvalidHandle;
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

		m_light->prepareShadowMap(m_lightView, m_lightProj, DepthImpl::Linear == m_depthImpl, m_currentShadowMapSize);

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
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, m_lightView[0], m_lightProj[AgLight::ProjType::Horizontal]);

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
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, m_lightView[AgPointLight::TetrahedronFaces::Green], m_lightProj[AgLight::ProjType::Horizontal]);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, m_lightView[AgPointLight::TetrahedronFaces::Yellow], m_lightProj[AgLight::ProjType::Horizontal]);

			if (pointLight->m_stencilPack)
			{
				const uint16_t f = m_currentShadowMapSize;   //full size
				const uint16_t h = m_currentShadowMapSize / 2; //half size
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, 0, 0, f, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, 0, h, f, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, 0, 0, h, f);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, h, 0, h, f);

				bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, m_lightView[AgPointLight::TetrahedronFaces::Blue], m_lightProj[AgLight::ProjType::Vertical]);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, m_lightView[AgPointLight::TetrahedronFaces::Red], m_lightProj[AgLight::ProjType::Vertical]);
			}
			else
			{
				const uint16_t h = m_currentShadowMapSize / 2; //half size
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, 0, 0, h, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, h, 0, h, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, 0, h, h, h);
				bgfx::setViewRect(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, h, h, h, h);

				bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, m_lightView[AgPointLight::TetrahedronFaces::Blue], m_lightProj[AgLight::ProjType::Horizontal]);
				bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, m_lightView[AgPointLight::TetrahedronFaces::Red], m_lightProj[AgLight::ProjType::Horizontal]);
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

			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_1_ID, m_lightView[0], m_lightProj[0]);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_2_ID, m_lightView[0], m_lightProj[1]);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_3_ID, m_lightView[0], m_lightProj[2]);
			bgfx::setViewTransform(AgRenderPass::E_VIEW_SHADOWMAP_4_ID, m_lightView[0], m_lightProj[3]);

			for (uint8_t ii = 0; ii < m_target_num; ++ii)
			{
				bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_SHADOWMAP_1_ID + ii, m_rtShadowMap[ii]);
			}

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

				for (uint8_t ii = 0; ii < m_target_num; ++ii)
				{
					bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_VBLUR_0_ID + ii * 2, m_rtBlur);         //vblur
					bgfx::setViewFrameBuffer(AgRenderPass::E_VIEW_HBLUR_0_ID + ii * 2, m_rtShadowMap[ii]); //hblur
				}
			}
		}
	}

	void AgShadowSystem::_ClearViews() const
	{
		if (!m_light)
			return;

		// Clear shadowmap rendertarget at beginning.
		const uint8_t flags0 = (typeid(AgDirectionalLight) == typeid(*m_light))
			? 0
			: BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL
			;

		bgfx::setViewClear(AgRenderPass::E_VIEW_SHADOWMAP_0_ID
			, flags0
			, 0xfefefefe //blur fails on completely white regions
			, 1.0f
			, 0
		);
		bgfx::touch(AgRenderPass::E_VIEW_SHADOWMAP_0_ID);

		const uint8_t flags1 = (typeid(AgDirectionalLight) == typeid(*m_light))
			? BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			: 0
			;

		for (uint8_t ii = 0; ii < 4; ++ii)
		{
			bgfx::setViewClear(AgRenderPass::E_VIEW_SHADOWMAP_1_ID + ii
				, flags1
				, 0xfefefefe //blur fails on completely white regions
				, 1.0f
				, 0
			);
			bgfx::touch(AgRenderPass::E_VIEW_SHADOWMAP_1_ID + ii);
		}
	}

	struct PosColorTexCoord0Vertex
	{
		float m_x;
		float m_y;
		float m_z;
		uint32_t m_rgba;
		float m_u;
		float m_v;

		static void init()
		{
			ms_decl
				.begin()
				.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
				.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
				.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
				.end();
		}

		static bgfx::VertexDecl ms_decl;
	};

	bgfx::VertexDecl PosColorTexCoord0Vertex::ms_decl;

	void screenSpaceQuad(float _texelHalfW, float _texelHalfH, bool _originBottomLeft = true, float _width = 1.0f, float _height = 1.0f)
	{
		if (3 == bgfx::getAvailTransientVertexBuffer(3, PosColorTexCoord0Vertex::ms_decl))
		{
			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, 3, PosColorTexCoord0Vertex::ms_decl);
			PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)vb.data;

			const float zz = 0.0f;

			const float minx = -_width;
			const float maxx = _width;
			const float miny = 0.0f;
			const float maxy = _height*2.0f;

			const float minu = -1.0f + _texelHalfW;
			const float maxu = 1.0f + _texelHalfW;

			float minv = _texelHalfH;
			float maxv = 2.0f + _texelHalfH;

			if (_originBottomLeft)
			{
				std::swap(minv, maxv);
				minv -= 1.0f;
				maxv -= 1.0f;
			}

			vertex[0].m_x = minx;
			vertex[0].m_y = miny;
			vertex[0].m_z = zz;
			vertex[0].m_rgba = 0xffffffff;
			vertex[0].m_u = minu;
			vertex[0].m_v = minv;

			vertex[1].m_x = maxx;
			vertex[1].m_y = miny;
			vertex[1].m_z = zz;
			vertex[1].m_rgba = 0xffffffff;
			vertex[1].m_u = maxu;
			vertex[1].m_v = minv;

			vertex[2].m_x = maxx;
			vertex[2].m_y = maxy;
			vertex[2].m_z = zz;
			vertex[2].m_rgba = 0xffffffff;
			vertex[2].m_u = maxu;
			vertex[2].m_v = maxv;

			bgfx::setVertexBuffer(0, &vb);
		}
	}

	void AgShadowSystem::_CraftStencilMask() const
	{
		AgShader* black_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SHADOW_BLACK);
		if (!black_shader)
			return;

		// Craft stencil mask for point light shadow map packing.
		AgPointLight* pointLight = dynamic_cast<AgPointLight*>(m_light);
		if (!pointLight || !pointLight->m_stencilPack)
			return;
	
		if (6 == bgfx::getAvailTransientVertexBuffer(6, m_posDecl))
		{
			struct Pos
			{
				float m_x, m_y, m_z;
			};

			bgfx::TransientVertexBuffer vb;
			bgfx::allocTransientVertexBuffer(&vb, 6, m_posDecl);
			Pos* vertex = (Pos*)vb.data;

			const float min = 0.0f;
			const float max = 1.0f;
			const float center = 0.5f;
			const float zz = 0.0f;

			vertex[0].m_x = min;
			vertex[0].m_y = min;
			vertex[0].m_z = zz;

			vertex[1].m_x = max;
			vertex[1].m_y = min;
			vertex[1].m_z = zz;

			vertex[2].m_x = center;
			vertex[2].m_y = center;
			vertex[2].m_z = zz;

			vertex[3].m_x = center;
			vertex[3].m_y = center;
			vertex[3].m_z = zz;

			vertex[4].m_x = max;
			vertex[4].m_y = max;
			vertex[4].m_z = zz;

			vertex[5].m_x = min;
			vertex[5].m_y = max;
			vertex[5].m_z = zz;

			bgfx::setState(0);
			bgfx::setStencil(BGFX_STENCIL_TEST_ALWAYS
				| BGFX_STENCIL_FUNC_REF(1)
				| BGFX_STENCIL_FUNC_RMASK(0xff)
				| BGFX_STENCIL_OP_FAIL_S_REPLACE
				| BGFX_STENCIL_OP_FAIL_Z_REPLACE
				| BGFX_STENCIL_OP_PASS_Z_REPLACE
			);
			bgfx::setVertexBuffer(0, &vb);
			bgfx::submit(AgRenderPass::E_VIEW_SHADOWMAP_0_ID, black_shader->m_program);
		}
	}

	void AgShadowSystem::drawPackDepth(const AgRenderNode* node)
	{
		if (!node || !m_light)
			return;

		// Craft shadow map.
		AgPointLight* pointLight = dynamic_cast<AgPointLight*>(m_light);
		// Draw scene into shadowmap.
		for (uint8_t ii = 0; ii < m_target_num; ++ii)
		{
			const uint8_t viewId = AgRenderPass::E_VIEW_SHADOWMAP_1_ID + ii;

			m_renderStateIndex = ShadowRenderState::ShadowMap_PackDepth;
			if (pointLight && pointLight->m_stencilPack)
			{
				m_renderStateIndex = (ii < 2) ? ShadowRenderState::ShadowMap_PackDepthHoriz : ShadowRenderState::ShadowMap_PackDepthVert;
			}

			// Draw shadow
			ViewIdArray views;
			views.push_back(viewId);
			node->draw(views, this, /*occlusionCulling*/-2);
		}
	}

	/*virtual*/
	bool AgShadowSystem::needTexture() const
	{
		return ShadowRenderState::Default == m_renderStateIndex;
	}

	/*virtual*/
	AgShader::Handle AgShadowSystem::getOverrideShader() const
	{
		return (ShadowRenderState::Default == m_renderStateIndex) ? m_currentLightingShader : m_currentPackDepthShader;
	}

	/*virtual*/
	AgRenderState AgShadowSystem::getOverrideStates() const
	{
		return s_renderStates[m_renderStateIndex];
	}

	void AgShadowSystem::blurShadowMap() const
	{
		const bool isVSM = (SmImpl::VSM == m_smImpl);
		AgShader* vBlur_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(isVSM ? AgShader::E_SHADOW_VBLUR_VSM : AgShader::E_SHADOW_VBLUR);
		if (!vBlur_shader)
			return;

		AgShader* hBlur_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(isVSM ? AgShader::E_SHADOW_HBLUR_VSM : AgShader::E_SHADOW_HBLUR);
		if (!hBlur_shader)
			return;

		bool bVsmOrEsm = (SmImpl::VSM == m_smImpl) || (SmImpl::ESM == m_smImpl);
		if (!bVsmOrEsm || !bgfx::isValid(m_rtBlur))
			return;
		
		float currentShadowMapSizef = float(int16_t(m_currentShadowMapSize));

		bgfx::setTexture(4, m_shadowMap[0], bgfx::getTexture(m_rtShadowMap[0]));
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		screenSpaceQuad(m_texelHalf / currentShadowMapSizef, m_texelHalf / currentShadowMapSizef, m_flipV);
		bgfx::submit(AgRenderPass::E_VIEW_VBLUR_0_ID, vBlur_shader->m_program);

		bgfx::setTexture(4, m_shadowMap[0], bgfx::getTexture(m_rtBlur));
		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
		screenSpaceQuad(m_texelHalf / currentShadowMapSizef, m_texelHalf / currentShadowMapSizef, m_flipV);
		bgfx::submit(AgRenderPass::E_VIEW_HBLUR_0_ID, hBlur_shader->m_program);

		if (typeid(AgDirectionalLight) == typeid(*m_light))
		{
			for (uint8_t ii = 1, jj = 2; ii < m_target_num; ++ii, jj += 2)
			{
				const uint8_t viewId = AgRenderPass::E_VIEW_VBLUR_0_ID + jj;

				bgfx::setTexture(4, m_shadowMap[0], bgfx::getTexture(m_rtShadowMap[ii]));
				bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
				screenSpaceQuad(m_texelHalf / currentShadowMapSizef, m_texelHalf / currentShadowMapSizef, m_flipV);
				bgfx::submit(viewId, vBlur_shader->m_program);

				bgfx::setTexture(4, m_shadowMap[0], bgfx::getTexture(m_rtBlur));
				bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
				screenSpaceQuad(m_texelHalf / currentShadowMapSizef, m_texelHalf / currentShadowMapSizef, m_flipV);
				bgfx::submit(viewId + 1, hBlur_shader->m_program);
			}
		}
	}

	void AgShadowSystem::prepareShadowMatrix()
	{
		_SwitchLightingDrawState();

		// Setup shadow mtx.
		const float ymul = (m_flipV) ? 0.5f : -0.5f;
		float zadd = (DepthImpl::Linear == m_depthImpl) ? 0.0f : 0.5f;

		const float mtxBias[16] =
		{
			0.5f, 0.0f, 0.0f, 0.0f,
			0.0f, ymul, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.5f, 0.5f, zadd, 1.0f,
		};

		AgPointLight* pointLight = dynamic_cast<AgPointLight*>(m_light);
		AgDirectionalLight* directionalLight = dynamic_cast<AgDirectionalLight*>(m_light);
		if (pointLight)
		{
			const float s = (m_flipV) ? 1.0f : -1.0f; //sign
			zadd = (DepthImpl::Linear == m_depthImpl) ? 0.0f : 0.5f;

			const float mtxCropBias[2][AgPointLight::TetrahedronFaces::Count][16] =
			{
				{ // settings.m_stencilPack == false

					{ // D3D: Green, OGL: Blue
						0.25f,    0.0f, 0.0f, 0.0f,
						0.0f, s*0.25f, 0.0f, 0.0f,
						0.0f,    0.0f, 0.5f, 0.0f,
						0.25f,   0.25f, zadd, 1.0f,
					},
					{ // D3D: Yellow, OGL: Red
						0.25f,    0.0f, 0.0f, 0.0f,
						0.0f, s*0.25f, 0.0f, 0.0f,
						0.0f,    0.0f, 0.5f, 0.0f,
						0.75f,   0.25f, zadd, 1.0f,
					},
					{ // D3D: Blue, OGL: Green
						0.25f,    0.0f, 0.0f, 0.0f,
						0.0f, s*0.25f, 0.0f, 0.0f,
						0.0f,    0.0f, 0.5f, 0.0f,
						0.25f,   0.75f, zadd, 1.0f,
					},
					{ // D3D: Red, OGL: Yellow
						0.25f,    0.0f, 0.0f, 0.0f,
						0.0f, s*0.25f, 0.0f, 0.0f,
						0.0f,    0.0f, 0.5f, 0.0f,
						0.75f,   0.75f, zadd, 1.0f,
					},
				},
				{ // settings.m_stencilPack == true

					{ // D3D: Red, OGL: Blue
						0.25f,   0.0f, 0.0f, 0.0f,
						0.0f, s*0.5f, 0.0f, 0.0f,
						0.0f,   0.0f, 0.5f, 0.0f,
						0.25f,   0.5f, zadd, 1.0f,
					},
					{ // D3D: Blue, OGL: Red
						0.25f,   0.0f, 0.0f, 0.0f,
						0.0f, s*0.5f, 0.0f, 0.0f,
						0.0f,   0.0f, 0.5f, 0.0f,
						0.75f,   0.5f, zadd, 1.0f,
					},
					{ // D3D: Green, OGL: Green
						0.5f,    0.0f, 0.0f, 0.0f,
						0.0f, s*0.25f, 0.0f, 0.0f,
						0.0f,    0.0f, 0.5f, 0.0f,
						0.5f,   0.75f, zadd, 1.0f,
					},
					{ // D3D: Yellow, OGL: Yellow
						0.5f,    0.0f, 0.0f, 0.0f,
						0.0f, s*0.25f, 0.0f, 0.0f,
						0.0f,    0.0f, 0.5f, 0.0f,
						0.5f,   0.25f, zadd, 1.0f,
					},
				}
			};

			//Use as: [stencilPack][flipV][tetrahedronFace]
			static const uint8_t cropBiasIndices[2][2][4] =
			{
				{ // settings.m_stencilPack == false
					{ 0, 1, 2, 3 }, //flipV == false
					{ 2, 3, 0, 1 }, //flipV == true
				},
				{ // settings.m_stencilPack == true
					{ 3, 2, 0, 1 }, //flipV == false
					{ 2, 3, 0, 1 }, //flipV == true
				},
			};

			for (uint8_t ii = 0; ii < AgPointLight::TetrahedronFaces::Count; ++ii)
			{
				AgLight::ProjType::Enum projType = (pointLight->m_stencilPack) ? AgLight::ProjType::Enum(ii > 1) : AgLight::ProjType::Horizontal;
				uint8_t biasIndex = cropBiasIndices[pointLight->m_stencilPack][uint8_t(m_flipV)][ii];

				float mtxTmp[16];
				bx::mtxMul(mtxTmp, pointLight->m_mtxYpr[ii], m_lightProj[projType]);
				bx::mtxMul(m_shadowMapMtx[ii], mtxTmp, mtxCropBias[pointLight->m_stencilPack][biasIndex]); //mtxYprProjBias
			}

			bx::mtxTranslate(m_mtxShadow //lightInvTranslate
				, -pointLight->m_position.m_v[0]
				, -pointLight->m_position.m_v[1]
				, -pointLight->m_position.m_v[2]
			);
		}
		else if(directionalLight)
		{
			for (uint8_t ii = 0; ii < directionalLight->m_numSplits; ++ii)
			{
				float mtxTmp[16];

				bx::mtxMul(mtxTmp, m_lightProj[ii], mtxBias);
				bx::mtxMul(m_shadowMapMtx[ii], m_lightView[0], mtxTmp); //lViewProjCropBias
			}
		}
		else /*if (LightType::SpotLight == m_settings.m_lightType)*/
		{
			float mtxTmp[16];
			bx::mtxMul(mtxTmp, m_lightProj[AgLight::ProjType::Horizontal], mtxBias);
			bx::mtxMul(m_mtxShadow, m_lightView[0], mtxTmp); //lightViewProjBias
		}
	}
	
	/*virtual*/
	void AgShadowSystem::begin()
	{
		_SetPerFrameUniforms();
		_UpdateLightTransform();
		_ClearViews();
		_CraftStencilMask();
	}

	/*virtual*/
	void AgShadowSystem::end()
	{
	}
}