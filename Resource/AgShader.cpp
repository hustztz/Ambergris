#include "AgShader.h"
#include "AgShaderUtils.h"

#include <bx/readerwriter.h>


namespace ambergris {

	AgShaderManager::AgShaderManager()
	{
	}

	AgShaderManager::~AgShaderManager()
	{
	}

	bool AgShaderManager::loadShader(bx::FileReaderI* _reader)
	{
		if (!_reader)
			return false;

		bool ret = true;
		for (int id = 0; id < GetSize(); id++)
		{
			AgShader* shader = Get(id);
			if (!shader)
				continue;

			switch (id)
			{
			case AgShader::E_LAMBERT_SHADER:
				shader->m_program = shaderUtils::loadProgram(_reader, "vs_mesh", "fs_mesh");
				ret &= bgfx::isValid(shader->m_program);
				break;
			case AgShader::E_INSTANCE_SHADER:
				shader->m_program = shaderUtils::loadProgram(_reader, "vs_instancing", "fs_instancing");
				ret &= bgfx::isValid(shader->m_program);
				break;
			default:
				break;
			}
		}

		return ret;
	}

	void AgShaderManager::unloadShader()
	{
		for (int id = 0; id < GetSize(); id++)
		{
			AgShader* shader = Get(id);
			if (!shader)
				continue;
			// Cleanup.
			if (bgfx::isValid(shader->m_program))
			{
				bgfx::destroy(shader->m_program);
			}
		}
	}
}