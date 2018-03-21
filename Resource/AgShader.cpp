#include "AgShader.h"
#include "AgShaderUtils.h"
#include "AgRenderResourceManager.h"

#include <bx/readerwriter.h>

namespace ambergris {

	bool AgShaderManager::loadBasicShader()
	{
		{
			AgShader* mesh_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_MESH_SHADING);
			if (!mesh_shader)
				return false;

			if (!mesh_shader->m_prepared)
			{
				mesh_shader->m_program = shaderUtils::loadProgram("vs_mesh", "fs_lambert");
				if (!bgfx::isValid(mesh_shader->m_program))
					return false;

				mesh_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);
				mesh_shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
				mesh_shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;
				mesh_shader->m_prepared = true;
			}
		}

		{
			AgShader* simple_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SIMPLE_SHADER);
			if (!simple_shader)
				return false;

			if (!simple_shader->m_prepared)
			{
				simple_shader->m_program = shaderUtils::loadProgram("vs_mesh", "fs_simple");
				if (!bgfx::isValid(simple_shader->m_program))
					return false;
				simple_shader->m_prepared = true;
			}
		}
		
		{
			AgShader* mesh_instance_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_MESH_INSTANCE_SHADER);
			if (!mesh_instance_shader)
				return false;

			if (!mesh_instance_shader->m_prepared)
			{
				mesh_instance_shader->m_program = shaderUtils::loadProgram("vs_instancing", "fs_lambert");
				if (!bgfx::isValid(mesh_instance_shader->m_program))
					return false;

				mesh_instance_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_lightDir", bgfx::UniformType::Vec4);
				mesh_instance_shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
				mesh_instance_shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;
				mesh_instance_shader->m_prepared = true;
			}
		}

		{
			AgShader* simple_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SIMPLE_INSTANCE_SHADER);
			if (!simple_shader)
				return false;

			if (!simple_shader->m_prepared)
			{
				simple_shader->m_program = shaderUtils::loadProgram("vs_instancing", "fs_simple");
				if (!bgfx::isValid(simple_shader->m_program))
					return false;
				simple_shader->m_prepared = true;
			}
		}
		return true;
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