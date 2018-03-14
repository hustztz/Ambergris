#include "AgRenderProxyNode.h"
#include "AgRenderItem.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgFxSystem.h"

#include <assert.h>

namespace ambergris {

	/*virtual*/
	bool AgRenderProxyNode::appendGeometry(
		const float* transform,
		const uint32_t* pick_id,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		return false;
	}

	/*virtual*/
	void AgRenderProxyNode::draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery)
	{
		if (!m_pItem)
			return;

		if (!bgfx::isValid(m_pItem->m_vbh) || !bgfx::isValid(m_pItem->m_ibh))
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

		if (!pFxSystem || pFxSystem->needTexture())
			_SubmitTexture(shader);

		if (pFxSystem)
		{
			pFxSystem->setOverrideResource(shader, nullptr);
		}
		_SubmitUniform(shader, m_pItem);

		bgfx::setState(shaderState);
		m_pItem->submit();

		bgfx::ProgramHandle progHandle = shader->m_program;

		if (AgShader::E_SIMPLE_SHADER == pFxSystem->getOverrideShader() && bgfx::isValid(m_pItem->m_oqh))
		{
			for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
			{
				bgfx::setCondition(m_pItem->m_oqh, true);//TODO
				bgfx::submit(*view, progHandle, 0, view != viewEnd - 1);
			}
		}
		else if (inOcclusionQuery && bgfx::isValid(m_pItem->m_oqh))
		{
			for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
			{
				bgfx::submit(*view, progHandle, m_pItem->m_oqh, 0, view != viewEnd - 1);
			}
		}
		else
		{
			for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
			{
				bgfx::submit(*view, progHandle, 0, view != viewEnd - 1);
			}
		}
	}

	/*virtual*/
	const AgRenderItem* AgRenderProxyNode::findItem(const uint32_t* pick_id) const
	{
		return nullptr;
	}

	
}