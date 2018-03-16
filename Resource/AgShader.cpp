#include "AgShader.h"
#include "AgShaderUtils.h"
#include "AgRenderResourceManager.h"

#include <bx/readerwriter.h>

#define AMBERGRIS_TEXTURE_STATE_FILTER (0            \
			| BGFX_TEXTURE_U_CLAMP       \
			| BGFX_TEXTURE_V_CLAMP     \
			| BGFX_TEXTURE_W_CLAMP     \
			| BGFX_TEXTURE_MIN_POINT     \
			| BGFX_TEXTURE_MIP_POINT     \
			| BGFX_TEXTURE_MAG_POINT     \
			)

namespace ambergris {

	bool AgShaderManager::loadShader()
	{
		bool ret = true;
		for (int id = 0; id < getSize(); id++)
		{
			AgShader* shader = get(id);
			if (!shader)
				continue;

			switch (id)
			{
			case AgShader::E_MESH_SHADING:
				shader->m_program = shaderUtils::loadProgram("vs_mesh", "fs_lambert");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);
					shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
					shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;
				}
				else
				{
					ret &= false;
				}
				break;
			case AgShader::E_SIMPLE_SHADER:
				shader->m_program = shaderUtils::loadProgram("vs_mesh", "fs_simple");
				ret &= bgfx::isValid(shader->m_program);
				break;
			case AgShader::E_SKY_LANDSCAPE_SHADER:
				shader->m_program = shaderUtils::loadProgram("vs_mesh", "fs_sky_landscape");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
					shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;
					shader->m_texture_slots[1].uniform_handle = bgfx::createUniform("s_texLightmap", bgfx::UniformType::Int1);
					shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
					shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_sunLuminance", bgfx::UniformType::Vec4);
					shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_skyLuminance", bgfx::UniformType::Vec4);
					shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_parameters", bgfx::UniformType::Vec4);
				}
				else
				{
					ret &= false;
				}
				break;
			case AgShader::E_PICKING_SHADER:
				shader->m_program = shaderUtils::loadProgram("vs_picking", "fs_picking_id");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_id", bgfx::UniformType::Vec4); // ID for drawing into ID buffer
				}
				else
				{
					ret &= false;
				}
				break;

			case AgShader::E_MESH_INSTANCE_SHADER:
				shader->m_program = shaderUtils::loadProgram("vs_instancing", "fs_lambert");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);
					shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
					shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;
				}
				else
				{
					ret &= false;
				}
				break;
			case AgShader::E_SIMPLE_INSTANCE_SHADER:
				shader->m_program = shaderUtils::loadProgram("vs_instancing", "fs_simple");
				ret &= bgfx::isValid(shader->m_program);
				break;
			case AgShader::E_SKY_LANDSCAPE_INSTANCE_SHADER:
				shader->m_program = shaderUtils::loadProgram("vs_instancing", "fs_sky_landscape");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
					shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;
					shader->m_texture_slots[1].uniform_handle = bgfx::createUniform("s_texLightmap", bgfx::UniformType::Int1);
					shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
					shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_sunLuminance", bgfx::UniformType::Vec4);
					shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_skyLuminance", bgfx::UniformType::Vec4);
					shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_parameters", bgfx::UniformType::Vec4);
				}
				else
				{
					ret &= false;
				}
				break;
			case AgShader::E_PICKING_INSTANCE_SHADER:
				shader->m_program = shaderUtils::loadProgram("vs_pick_instancing", "fs_picking_id");
				ret &= (bgfx::isValid(shader->m_program));
				break;
			case AgShader::E_SKY:
				shader->m_program = shaderUtils::loadProgram("vs_sky", "fs_sky");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
					shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_skyLuminanceXYZ", bgfx::UniformType::Vec4);
					shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_parameters", bgfx::UniformType::Vec4);
					shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_perezCoeff", bgfx::UniformType::Vec4, 5);
					shader->m_uniforms[4].uniform_handle = bgfx::createUniform("u_sunLuminance", bgfx::UniformType::Vec4);
				}
				else
				{
					ret &= false;
				}
				break;
			case AgShader::E_SKY_COLOR_BANDING:
				shader->m_program = shaderUtils::loadProgram("vs_sky", "fs_sky_color_banding_fix");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
					shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_skyLuminanceXYZ", bgfx::UniformType::Vec4);
					shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_parameters", bgfx::UniformType::Vec4);
					shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_perezCoeff", bgfx::UniformType::Vec4, 5);
					shader->m_uniforms[4].uniform_handle = bgfx::createUniform("u_sunLuminance", bgfx::UniformType::Vec4);
				}
				else
				{
					ret &= false;
				}
				break;
			default:
				break;
			}
		}

		return ret;
	}

	void AgShaderManager::unloadShader()
	{
		for (int id = 0; id < getSize(); id++)
		{
			AgShader* shader = get(id);
			if (!shader)
				continue;
			// Cleanup.
			if (bgfx::isValid(shader->m_program))
			{
				bgfx::destroy(shader->m_program);
				shader->m_program = BGFX_INVALID_HANDLE;
			}
			for (uint8_t i = 0; i < AgShader::MAX_UNIFORM_COUNT; i++)
			{
				if (bgfx::isValid(shader->m_uniforms[i].uniform_handle))
				{
					bgfx::destroy(shader->m_uniforms[i].uniform_handle);
				}
				shader->m_uniforms[i].uniform_handle = BGFX_INVALID_HANDLE;
			}
			for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; i++)
			{
				if (bgfx::isValid(shader->m_texture_slots[i].uniform_handle))
				{
					bgfx::destroy(shader->m_texture_slots[i].uniform_handle);
				}
				shader->m_texture_slots[i].uniform_handle = BGFX_INVALID_HANDLE;
			}
		}
	}

}