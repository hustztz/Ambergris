#include "BgfxMaterialManager.h"
#include "BgfxShaderManager.h"

namespace ambergris_bgfx {

	void BgfxMaterialManager::Destroy()
	{
		Singleton<ambergris_bgfx::BgfxShaderManager>::instance().unloadShader();
	}
	
	void BgfxMaterialManager::Init(bx::FileReaderI* _reader)
	{
		// Create program from shaders.
		if (!Singleton<ambergris_bgfx::BgfxShaderManager>::instance().loadShader(_reader))
			return;

		for (int id = 0; id < E_COUNT; id++)
		{
			switch (id)
			{
			case E_LAMBERT:
				m_mat[id].m_state_flags = 0
					| BGFX_STATE_RGB_WRITE
					| BGFX_STATE_ALPHA_WRITE
					| BGFX_STATE_DEPTH_WRITE
					| BGFX_STATE_DEPTH_TEST_LESS
					| BGFX_STATE_CULL_CCW
					| BGFX_STATE_MSAA;
				m_mat[id].m_program = Singleton<BgfxShaderManager>::instance().m_mesh_program;
				break;
			case E_PHONG:
				break;
			default:
				break;
			}
		}
		
	}

	uint64_t BgfxMaterialManager::GetRenderState(MaterialIndex id) const
	{
		if (id < 0 || id >= E_COUNT)
			return 0;
		return m_mat[id].m_state_flags;
	}

	bgfx::ProgramHandle BgfxMaterialManager::GetProgramHandle(MaterialIndex id) const
	{
		if (id < 0 || id >= E_COUNT)
			return BGFX_INVALID_HANDLE;
		return m_mat[id].m_program;
	}
}