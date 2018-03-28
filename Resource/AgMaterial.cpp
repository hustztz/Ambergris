#include "AgMaterial.h"
#include "Resource/AgRenderResourceManager.h"

namespace ambergris {

	AgMaterial::AgMaterial()
		: AgResource()
		, m_name("default")
	{
		m_ka.m_r = m_ka.m_g = m_ka.m_b = 1.0f; m_ka.m_unused = 0.0f; //ambient
		m_kd.m_r = m_kd.m_g = m_kd.m_b = 1.0f; m_kd.m_unused = 0.0f; //diffuse
		m_ks.m_r = m_ks.m_g = m_ks.m_b = 1.0f; m_ks.m_ns = 0.0f; //specular, exponent
	}

	void AgMaterialManager::destroy()
	{
		
	}

	void AgMaterialManager::init()
	{
		for (int id = 0; id < AgMaterial::E_COUNT; id++)
		{
			AgMaterial* mat = get(id);
			if(!mat || AgMaterial::kInvalidHandle != mat->m_handle)
				continue;

			switch (id)
			{
			case AgMaterial::E_LAMBERT:
				/*mat->m_state_flags = 0
					| BGFX_STATE_WRITE_RGB
					| BGFX_STATE_WRITE_A
					| BGFX_STATE_WRITE_Z
					| BGFX_STATE_DEPTH_TEST_LESS
					| BGFX_STATE_CULL_CCW
					| BGFX_STATE_MSAA;
				mat->m_shader = AgShader::E_MESH_SHADING;*/
				mat->m_name = "Lambert";
				break;
			case AgMaterial::E_PHONG:
				break;
			default:
				break;
			}

			mat->m_handle = id;
		}
	}

}