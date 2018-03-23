#include "AgRenderNode.h"
#include "AgRenderItem.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgRenderPass.h"
#include "AgFxSystem.h"

#include <assert.h>

namespace ambergris {

	/*virtual*/
	void AgRenderNode::destroy()
	{
		for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; i++)
		{
			m_texture[i] = AgTexture::kInvalidHandle;
		}
	}

	void AgRenderNode::setTexture(uint8_t slot, AgTexture::Handle tex_handle)
	{
		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_MESH_SHADING);//TODO
		if (!shader)
			return;

		if (slot >= shader->getTextureSlotSize())
			return;

		m_texture[slot] = tex_handle;
	}

	void AgRenderNode::_SubmitTexture(const AgShader* shader) const
	{
		if (!shader)
			return;

		for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; ++i)
		{
			const AgTexture* tex = Singleton<AgRenderResourceManager>::instance().m_textures.get(m_texture[i]);
			if (!tex)
				continue;

			if (!bgfx::isValid(shader->m_texture_slots[i].uniform_handle))
				break;

			bgfx::setTexture(i, shader->m_texture_slots[i].uniform_handle, tex->m_tex_handle, shader->m_texture_slots[i].texture_state);
		}
	}

	void AgRenderNode::_SubmitUniform(const AgShader* shader, const AgRenderItem*	item) const
	{
		if (!shader || !item)
			return;

		for (uint16_t i = 0; i < AgShader::MAX_UNIFORM_COUNT; ++i)
		{
			if (!bgfx::isValid(shader->m_uniforms[i].uniform_handle) || !item->m_uniformData[i].data /*|| !item->m_uniformData[i].dirty*/)
				break;
			bgfx::setUniform(shader->m_uniforms[i].uniform_handle, item->m_uniformData[i].data);
			//item->m_uniformData[i].dirty = false;
		}
	}

}