#include "AgMaterial.h"

namespace ambergris {

	bgfx::ProgramHandle AgMaterial::getProgramHandle() const
	{
		const AgShader* shader = Singleton<AgShaderManager>::instance().get(m_shader);
		if (!shader)
			return BGFX_INVALID_HANDLE;

		return shader->m_program;
	}

	uint8_t	AgMaterial::getTextureSlotSize() const
	{
		const AgShader* shader = Singleton<AgShaderManager>::instance().get(m_shader);
		if (!shader)
			return 0;

		return shader->m_texture_slot_count;
	}

	const AgShader::TextureSlot* AgMaterial::getTextureSlot(uint8_t slot) const
	{
		const AgShader* shader = Singleton<AgShaderManager>::instance().get(m_shader);
		if (!shader)
			return nullptr;

		if (slot >= shader->m_texture_slot_count)
			return nullptr;

		return &shader->m_texture_slots[slot];
	}

	void AgMaterialManager::destroy()
	{
		Singleton<AgShaderManager>::instance().unloadShader();
	}

	void AgMaterialManager::init(bx::FileReaderI* _reader)
	{
		// Create program from shaders.
		if (!Singleton<AgShaderManager>::instance().loadShader(_reader))
			return;

		for (int id = 0; id < AgMaterial::E_COUNT; id++)
		{
			AgMaterial* mat = get(id);
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

}