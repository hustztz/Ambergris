#include "AgMaterial.h"
#include "Resource/AgRenderResourceManager.h"

namespace ambergris {

	void AgMaterialManager::destroy()
	{
		
	}

	void AgMaterialManager::init()
	{
		for (int id = 0; id < AgMaterial::E_COUNT; id++)
		{
			AgMaterial* mat = get(id);
			if(!mat)
				continue;
			switch (id)
			{
			case AgMaterial::E_LAMBERT:
				mat->m_state_flags = 0
					| BGFX_STATE_WRITE_RGB
					| BGFX_STATE_WRITE_A
					| BGFX_STATE_WRITE_Z
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

}