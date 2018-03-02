#include "AgShader.h"
#include "AgShaderUtils.h"
#include "AgRenderResourceManager.h"

#include <bx/readerwriter.h>


namespace ambergris {

	bool AgShaderManager::loadShader(bx::FileReaderI* _reader)
	{
		if (!_reader)
			return false;

		bool ret = true;
		for (int id = 0; id < getSize(); id++)
		{
			AgShader* shader = get(id);
			if (!shader)
				continue;

			switch (id)
			{
			case AgShader::E_LAMBERT_SHADER:
				shader->m_program = shaderUtils::loadProgram(_reader, "vs_lambert", "fs_lambert");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
				}
				else
				{
					ret &= false;
				}
				break;
			case AgShader::E_INSTANCE_SHADER:
				shader->m_program = shaderUtils::loadProgram(_reader, "vs_instancing", "fs_instancing");
				ret &= bgfx::isValid(shader->m_program);
				break;
			case AgShader::E_PICKING_SHADER:
				shader->m_program = shaderUtils::loadProgram(_reader, "vs_picking", "fs_picking_id");
				if (bgfx::isValid(shader->m_program))
				{
					shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_id", bgfx::UniformType::Vec4); // ID for drawing into ID buffer
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