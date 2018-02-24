#include "AgMaterial.h"
#include "AgShader.h"

namespace ambergris {

	void AgMaterialManager::Destroy()
	{
		Singleton<AgShaderManager>::instance().unloadShader();
	}

	void AgMaterialManager::Init(bx::FileReaderI* _reader)
	{
		// Create program from shaders.
		if (!Singleton<AgShaderManager>::instance().loadShader(_reader))
			return;

		for (int id = 0; id < AgMaterial::E_COUNT; id++)
		{
			AgMaterial* mat = Get(id);
			if(!mat)
				continue;
			switch (id)
			{
			case AgMaterial::E_LAMBERT:
				mat->m_state_flags = 0
					| BGFX_STATE_RGB_WRITE
					| BGFX_STATE_ALPHA_WRITE
					| BGFX_STATE_DEPTH_WRITE
					| BGFX_STATE_DEPTH_TEST_LESS
					| BGFX_STATE_CULL_CCW
					| BGFX_STATE_MSAA;
				mat->m_shader = AgShader::E_LAMBERT_SHADER;
				mat->m_name = "Lambert";
				break;
			case AgMaterial::E_PHONG:
				break;
			default:
				break;
			}
		}

	}

	uint64_t AgMaterialManager::GetRenderState(AgMaterial::MaterialType id) const
	{
		if (id < 0 || id >= AgMaterial::E_COUNT)
			return 0;
		const AgMaterial* mat = Get(id);
		if (!mat)
			return 0;
		return mat->m_state_flags;
	}

	bgfx::ProgramHandle AgMaterialManager::GetProgramHandle(AgMaterial::MaterialType id) const
	{
		if (id < 0 || id >= AgMaterial::E_COUNT)
			return BGFX_INVALID_HANDLE;
		const AgMaterial* mat = Get(id);
		if (!mat)
			return BGFX_INVALID_HANDLE;
		const AgShader* shader = Singleton<AgShaderManager>::instance().Get(mat->m_shader);
		if (!shader)
			return BGFX_INVALID_HANDLE;

		return shader->m_program;
	}
}