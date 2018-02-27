#include "AgRenderNode.h"

#include <assert.h>

namespace ambergris {

	void AgRenderNode::destroy()
	{
		for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; i++)
		{
			m_texture[i] = AgTexture::kInvalidHandle;
		}
		for each (auto item in m_items)
		{
			item.destroyBuffers();
		}
		m_items.clear();
	}

	bool AgRenderNode::appendGeometry(
		const float* transform,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		if (!vertBuf || 0 == vertSize || !indexBuf || 0 == indexSize)
			return false;

		if (m_items.empty())
			m_decl = decl;
		else
		{
			assert(decl.getStride() == m_decl.getStride());
			if (decl.getStride() != m_decl.getStride())
				return false;
		}

		AgRenderItem renderItem(decl,
			vertBuf, vertSize,
			indexBuf, indexSize);
		renderItem.setTransform(transform);

		m_items.push_back(renderItem);
		return true;
	}

	void AgRenderNode::setTexture(uint8_t slot, AgTexture::Handle tex_handle)
	{
		const AgMaterial* mat = Singleton<AgMaterialManager>::instance().get(m_material_handle);
		if (!mat)
			return;

		if (slot >= mat->getTextureSlotSize())
			return;

		m_texture[slot] = tex_handle;
	}

	void AgRenderNode::draw(bgfx::ViewId view) const
	{
		const AgMaterial* mat = Singleton<AgMaterialManager>::instance().get(m_material_handle);
		if (!mat)
			return;

		uint64_t state = mat->m_state_flags;
		bgfx::ProgramHandle progHandle = mat->getProgramHandle();
		bgfx::setState(state);
		for (uint8_t i = 0; i < mat->getTextureSlotSize(); ++i)
		{
			const AgTexture* tex = Singleton<AgTextureManager>::instance().get(m_texture[i]);
			if (!tex)
				continue;

			const AgShader::TextureSlot* tex_slot = mat->getTextureSlot(i);
			if (!tex_slot)
				continue;
			
			bgfx::setTexture(i, tex_slot->uniform_handle, tex->m_tex_handle, tex_slot->texture_state);
		}
		for (AgRenderNode::RenderItemArray::const_iterator it = m_items.begin(), itEnd = m_items.end(); it != itEnd; ++it)
		{
			it->submitBuffers();
			bgfx::submit(view, progHandle, 0, it != itEnd - 1);
		}
	}
}