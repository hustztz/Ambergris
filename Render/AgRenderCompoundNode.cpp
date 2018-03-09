#include "AgRenderCompoundNode.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgRenderPass.h"
#include "AgFxSystem.h"

#include <assert.h>

namespace ambergris {

	/*virtual*/
	void AgRenderCompoundNode::destroy()
	{
		for(auto item= m_items.begin(); item != m_items.end(); ++ item)
		{
			item->destroyBuffers();
		}
		m_items.clear();
		AgRenderNode::destroy();
	}

	/*virtual*/
	bool AgRenderCompoundNode::appendGeometry(
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
		AgRenderItem renderItem;
		renderItem.setBuffers(decl,
			vertBuf, vertSize,
			indexBuf, indexSize);
		renderItem.setTransform(transform);
		renderItem.setPickID(pick_id);

		m_items.push_back(renderItem);
		return true;
	}

	/*virtual*/
	void AgRenderCompoundNode::draw(bgfx::ViewId view, AgFxSystem* pFxSystem, bool inOcclusionQuery)
	{
		if (m_items.empty())
			return;

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

		if (AgRenderPass::E_PASS_ID != view)
		{
			if (!pFxSystem || pFxSystem->needTexture())
				_SubmitTexture(shader);

			if (pFxSystem)
			{
				pFxSystem->setOverrideResource(shader, nullptr);
			}
		}

		bgfx::setState(shaderState);

		bgfx::ProgramHandle progHandle = shader->m_program;
		for (AgRenderCompoundNode::RenderItemArray::iterator it = m_items.begin(), itEnd = m_items.end(); it != itEnd; ++it)
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
				_SubmitUniform(shader, &*it);
			}
			it->submit();
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

}