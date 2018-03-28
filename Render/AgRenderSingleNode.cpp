#include "AgRenderSingleNode.h"
#include "Resource/AgRenderResourceManager.h"

#include "AgFxSystem.h"

#include <assert.h>

namespace ambergris {

	/*virtual*/
	void AgRenderSingleNode::destroy()
	{
		AgRenderNode::destroy();
		m_item.destroyBuffers();
	}

	/*virtual*/
	bool AgRenderSingleNode::appendGeometry(
		const float* transform,
		AgMaterial::Handle material,
		AgBoundingbox::Handle bbox,
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
		m_item.setMaterial(material);
		m_item.setPickID(pick_id);
		m_item.enableOcclusionQuery();
		m_item.m_bbox = bbox;
		return true;
	}

	/*virtual*/
	void AgRenderSingleNode::draw(const ViewIdArray& views, const AgFxSystem* pFxSystem, int32_t occlusionCulling) const
	{
		if (!bgfx::isValid(m_item.m_vbh) || !bgfx::isValid(m_item.m_ibh))
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
			pFxSystem->setPerDrawUniforms(shader, &m_item);
		}
		_SubmitUniform(shader, &m_item);

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
		
		m_item.submit();

		bgfx::ProgramHandle progHandle = shader->m_program;

		if (occlusionCulling > 0 && bgfx::isValid(m_item.m_oqh))
		{
			uint32_t occlusion_threshold = m_item.m_occlusion_threshold * occlusionCulling;
			for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
			{
				bgfx::setCondition(m_item.m_oqh, true, occlusion_threshold);//TODO:multi-view
				bgfx::submit(*view, progHandle, 0, view != viewEnd - 1);
			}
		}
		else if (-1 == occlusionCulling && bgfx::isValid(m_item.m_oqh))
		{
			//occlusionQuery
			for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
			{
				bgfx::submit(*view, progHandle, m_item.m_oqh, 0, view != viewEnd - 1);
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
	const AgRenderItem* AgRenderSingleNode::getItem(uint16_t id) const
	{
		if (0 == id)
		{
			return &m_item;
		}
		return nullptr;
	}
}