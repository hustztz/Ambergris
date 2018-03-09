#include "AgRenderNode.h"
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
		m_item.destroyBuffers();
	}

	/*virtual*/
	bool AgRenderNode::appendGeometry(
		const float* transform,
		const uint32_t* pick_id,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		if (!vertBuf || 0 == vertSize || !indexBuf || 0 == indexSize)
			return false;

		m_decl = decl;
		m_item.setBuffers(decl,
			vertBuf, vertSize,
			indexBuf, indexSize);
		m_item.setTransform(transform);
		m_item.setPickID(pick_id);
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

		m_texture[slot] = tex_handle;
	}

	void AgRenderNode::_SubmitTexture(const AgShader* shader)
	{
		if (!shader)
			return;

		for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; ++i)
		{
			const AgTexture* tex = Singleton<AgRenderResourceManager>::instance().m_textures.get(m_texture[i]);
			if (!tex)
				continue;

			if (!bgfx::isValid(shader->m_texture_slots[i].uniform_handle))
				continue;

			bgfx::setTexture(i, shader->m_texture_slots[i].uniform_handle, tex->m_tex_handle, shader->m_texture_slots[i].texture_state);
		}
	}

	void AgRenderNode::_SubmitUniform(const AgShader* shader, AgRenderItem*	item)
	{
		if (!shader || !item)
			return;

		for (uint16_t i = 0; i < AgShader::MAX_UNIFORM_COUNT; ++i)
		{
			if (!bgfx::isValid(shader->m_uniforms[i].uniform_handle) || !item->m_uniformData[i].dirty)
				continue;
			bgfx::setUniform(shader->m_uniforms[i].uniform_handle, item->m_uniformData[i].data);
			item->m_uniformData[i].dirty = false;
		}
	}

	void AgRenderNode::draw(bgfx::ViewId view, AgFxSystem* pFxSystem, bool inOcclusionQuery)
	{
		if (!bgfx::isValid(m_item.m_vbh) || !bgfx::isValid(m_item.m_ibh))
			return;

		const AgShader* shader = nullptr;
		uint64_t shaderState = BGFX_STATE_DEFAULT;
		if (pFxSystem && AgShader::E_COUNT != pFxSystem->getOverrideShader())
		{
			shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(pFxSystem->getOverrideShader());
			shaderState = pFxSystem->getOverrideStates();
		}
		else
		{
			const AgMaterial* mat = Singleton<AgRenderResourceManager>::instance().m_materials.get(m_material_handle);
			if (!mat)
				return;

			shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(mat->getShaderHandle());
			shaderState = mat->m_state_flags;
		}
		
		if (!shader)
			return;

		if (AgRenderPass::E_PASS_ID == view && pFxSystem)
		{
			// Submit ID pass based on mesh ID
			float idsF[4];
			idsF[0] = m_item.m_pick_id[0] / 255.0f;
			idsF[1] = m_item.m_pick_id[1] / 255.0f;
			idsF[2] = m_item.m_pick_id[2] / 255.0f;
			idsF[3] = 1.0f;
			pFxSystem->setOverrideResource(shader, idsF);
		}
		else
		{
			if (!pFxSystem || pFxSystem->needTexture())
				_SubmitTexture(shader);

			if (pFxSystem)
			{
				pFxSystem->setOverrideResource(shader, nullptr);
			}
			_SubmitUniform(shader, &m_item);
		}

		bgfx::setState(shaderState);
		m_item.submit();

		bgfx::ProgramHandle progHandle = shader->m_program;

		if (AgShader::E_SIMPLE_SHADER == pFxSystem->getOverrideShader() && bgfx::isValid(m_item.m_oqh))
		{
			bgfx::setCondition(m_item.m_oqh, true);
			bgfx::submit(view, progHandle);
		}
		else if (inOcclusionQuery && bgfx::isValid(m_item.m_oqh))
		{
			bgfx::submit(view, progHandle, m_item.m_oqh);
		}
		else
		{
			bgfx::submit(view, progHandle);
		}
	}

}