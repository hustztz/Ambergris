#include "BgfxShaderManager.h"
#include "BgfxShaderUtils.h"

#include <bx/readerwriter.h>


namespace ambergris_bgfx {

	BgfxShaderManager::BgfxShaderManager()
		: m_mesh_program(BGFX_INVALID_HANDLE)
		, m_instancing_program(BGFX_INVALID_HANDLE)
	{
	}

	BgfxShaderManager::~BgfxShaderManager()
	{
	}

	bool BgfxShaderManager::loadShader(bx::FileReaderI* _reader)
	{
		if (!_reader)
			return false;

		m_mesh_program = shaderUtils::loadProgram(_reader, "vs_mesh", "fs_mesh");
		m_instancing_program = shaderUtils::loadProgram(_reader, "vs_instancing", "fs_instancing");

		return bgfx::isValid(m_mesh_program);
	}

	void BgfxShaderManager::unloadShader()
	{
		// Cleanup.
		if (bgfx::isValid(m_mesh_program))
		{
			bgfx::destroy(m_mesh_program);
		}
		if (bgfx::isValid(m_instancing_program))
		{
			bgfx::destroy(m_instancing_program);
		}
	}
}