#include "AgRenderProxyNode.h"
#include "AgRenderItem.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgFxSystem.h"

#include <assert.h>

namespace ambergris {

	/*virtual*/
	void AgRenderProxyNode::draw(const ViewIdArray& views, const AgFxSystem* pFxSystem, int32_t occlusionCulling) const
	{
		if (!m_pItem)
			return;

		if (!bgfx::isValid(m_pItem->m_vbh) || !bgfx::isValid(m_pItem->m_ibh))
			return;

		if (!pFxSystem || AgShader::E_COUNT == pFxSystem->getOverrideShader())
			return;

		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(pFxSystem->getOverrideShader());
		if (!shader || AgShader::kInvalidHandle == shader->m_handle)
			return;
		
		if (!pFxSystem || pFxSystem->needTexture())
			_SubmitTexture(shader);

		if (pFxSystem)
		{
			pFxSystem->setPerDrawUniforms(shader, nullptr);
		}
		_SubmitUniform(shader, m_pItem);

		if (pFxSystem->getOverrideStates().isDefaultState())
		{
			bgfx::setState(m_renderState.m_state, m_renderState.m_blendFactorRgba);
			bgfx::setStencil(m_renderState.m_fstencil, m_renderState.m_bstencil);
		}
		else
		{
			bgfx::setState(pFxSystem->getOverrideStates().m_state, pFxSystem->getOverrideStates().m_blendFactorRgba);
			bgfx::setStencil(pFxSystem->getOverrideStates().m_fstencil, pFxSystem->getOverrideStates().m_bstencil);
		}
		m_pItem->submit();
		// Override transform
		const AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_transform);
		if (transform)
		{
			float transformData[16];
			transform->getFloatTransform(transformData);
			bgfx::setTransform(transformData);
		}

		bgfx::ProgramHandle progHandle = shader->m_program;

		if (occlusionCulling > 0 && bgfx::isValid(m_pItem->m_oqh))
		{
			uint32_t occlusion_threshold = m_pItem->m_occlusion_threshold * occlusionCulling;
			for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
			{
				bgfx::setCondition(m_pItem->m_oqh, true, occlusion_threshold);//TODO
				bgfx::submit(*view, progHandle, 0, view != viewEnd - 1);
			}
		}
		else if (-1 == occlusionCulling && bgfx::isValid(m_pItem->m_oqh))
		{
			// occlusionQuery
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
	const AgRenderItem* AgRenderProxyNode::getItem(uint16_t id) const
	{
		if (0 == id)
		{
			return m_pItem;
		}
		return nullptr;
	}
}