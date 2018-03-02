#include "AgRenderNode.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgRenderPass.h"

#include <assert.h>

#define AMBERGRIS_STATE_OCCLUSION_QUERY (0            \
			| BGFX_STATE_DEPTH_TEST_LEQUAL       \
			| BGFX_STATE_CULL_CW     \
			)

namespace ambergris {

	void AgRenderNode::destroy()
	{
		for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; i++)
		{
			m_texture[i].handle = AgTexture::kInvalidHandle;
		}
		for each (auto item in m_items)
		{
			item.destroyBuffers();
		}
		m_items.clear();
	}

	bool AgRenderNode::appendGeometry(
		const float* transform,
		const uint32_t* pick_id,
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
		renderItem.setPickID(pick_id);

		m_items.push_back(renderItem);
		return true;
	}

	void AgRenderNode::setTexture(uint8_t slot, AgTexture::Handle tex_handle)
	{
		const AgMaterial* mat = Singleton<AgRenderResourceManager>::instance().m_materials.get(m_material_handle);
		if (!mat)
			return;

		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(mat->getShaderHandle());
		if (!shader)
			return;

		if (slot >= shader->getTextureSlotSize())
			return;

		m_texture[slot].handle = tex_handle;
		m_texture[slot].dirty = true;
	}

	void AgRenderNode::_SubmitTexture(const AgShader* shader)
	{
		if (!shader)
			return;

		for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; ++i)
		{
			const AgTexture* tex = Singleton<AgRenderResourceManager>::instance().m_textures.get(m_texture[i].handle);
			if (!tex)
				continue;

			if (!bgfx::isValid(shader->m_texture_slots[i].uniform_handle) /*|| !m_texture[i].dirty*/)
				continue;

			bgfx::setTexture(i, shader->m_texture_slots[i].uniform_handle, tex->m_tex_handle, shader->m_texture_slots[i].texture_state);
			m_texture[i].dirty = false;
		}
	}

	void AgRenderNode::draw(bgfx::ViewId view,
		AgShader::Handle shaderHandle,
		uint64_t state,
		bool inOcclusionQuery /*= false*/,
		bool needOcclusionCondition /*= false*/)
	{
		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(shaderHandle);
		if (!shader)
			return;

		_SubmitTexture(shader);

		bgfx::ProgramHandle progHandle = shader->m_program;
		bgfx::setState(state);
		for (AgRenderNode::RenderItemArray::iterator it = m_items.begin(), itEnd = m_items.end(); it != itEnd; ++it)
		{
			if (AgRenderPass::E_PASS_ID == view)
			{
				// Submit ID pass based on mesh ID
				float idsF[4];
				idsF[0] = it->m_pick_id[0] / 255.0f;
				idsF[1] = it->m_pick_id[1] / 255.0f;
				idsF[2] = it->m_pick_id[2] / 255.0f;
				idsF[3] = 1.0f;
				bgfx::setUniform(shader->m_uniforms[0].uniform_handle, idsF);
			}
			else
			{
				// Update Uniforms
				for (uint8_t i = 0; i < AgShader::MAX_UNIFORM_COUNT; ++i)
				{
					if (!bgfx::isValid(shader->m_uniforms[i].uniform_handle) || !it->m_uniformData[i].dirty)
						continue;
					bgfx::setUniform(shader->m_uniforms[i].uniform_handle, it->m_uniformData[i].data);
					it->m_uniformData[i].dirty = false;
				}
			}

			it->submit();
			if (inOcclusionQuery)
			{
				if (bgfx::isValid(it->m_oqh))
				{
					if (needOcclusionCondition)
					{
						bgfx::setState(AMBERGRIS_STATE_OCCLUSION_QUERY);
						bgfx::setCondition(it->m_oqh, true);
						bgfx::submit(view, progHandle, 0, it != itEnd - 1);
					}
					bgfx::submit(view, progHandle, it->m_oqh, 0, it != itEnd - 1);
				}
				else
				{
					bgfx::setCondition(BGFX_INVALID_HANDLE, false);
					bgfx::setState(state);
					bgfx::submit(view, progHandle, 0, it != itEnd - 1);
				}
			}
			else
			{
				bgfx::submit(view, progHandle, 0, it != itEnd - 1);
			}
		}
	}

	void AgRenderNode::draw(bgfx::ViewId view, bool inOcclusionQuery /*= false*/)
	{
		const AgMaterial* mat = Singleton<AgRenderResourceManager>::instance().m_materials.get(m_material_handle);
		if (!mat)
			return;
		if (m_items.empty())
			return;

		draw(view, mat->getShaderHandle(), mat->m_state_flags, inOcclusionQuery, true);
	}

}