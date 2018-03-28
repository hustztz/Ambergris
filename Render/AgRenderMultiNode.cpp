#include "AgRenderMultiNode.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgRenderItem.h"
#include "AgFxSystem.h"

#include <assert.h>

namespace ambergris {

	/*virtual*/
	void AgRenderMultiNode::destroy()
	{
		for(auto iter = m_items.begin(); iter != m_items.end(); ++iter)
		{
			AgRenderItem* item = *iter;
			if (item)
			{
				item->destroyBuffers();
				delete item;
			}
		}
		m_items.clear();
		AgRenderNode::destroy();
	}

	/*virtual*/
	bool AgRenderMultiNode::appendGeometry(
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

		if (m_items.empty())
			m_decl = decl;
		else
		{
			assert(decl.getStride() == m_decl.getStride());
			if (decl.getStride() != m_decl.getStride())
				return false;
		}
		AgRenderItem* renderItem = new AgRenderItem;
		renderItem->setBuffers(decl,
			vertBuf, vertSize,
			indexBuf, indexSize);
		renderItem->setTransform(transform);
		renderItem->setMaterial(material);
		renderItem->setPickID(pick_id);
		renderItem->enableOcclusionQuery();
		renderItem->m_bbox = bbox;

		m_items.push_back(renderItem);
		return true;
	}

	/*virtual*/
	void AgRenderMultiNode::draw(const ViewIdArray& views, const AgFxSystem* pFxSystem, int32_t occlusionCulling) const
	{
		if (m_items.empty())
			return;

		if (!pFxSystem || AgShader::E_COUNT == pFxSystem->getOverrideShader())
			return;

		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(pFxSystem->getOverrideShader());
		if (!shader || AgShader::kInvalidHandle == shader->m_handle)
			return;

		if (!pFxSystem || pFxSystem->needTexture())
			_SubmitTexture(shader);

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

		bgfx::ProgramHandle progHandle = shader->m_program;
		for (AgRenderMultiNode::RenderItemArray::const_iterator it = m_items.cbegin(), itEnd = m_items.cend(); it != itEnd; ++it)
		{
			const AgRenderItem* item = *it;
			if (!item)
				continue;

			if (pFxSystem)
				pFxSystem->setPerDrawUniforms(shader, item);
			_SubmitUniform(shader, item);

			item->submit();
			if (occlusionCulling > 0 && bgfx::isValid(item->m_oqh))
			{
				uint32_t occlusion_threshold = item->m_occlusion_threshold * occlusionCulling;
				for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
				{
					bgfx::setCondition(item->m_oqh, true, occlusion_threshold);//TODO:multi-view
					bgfx::submit(*view, progHandle, 0 , it != itEnd - 1 && view != viewEnd - 1);
				}
			}
			else if (-1 == occlusionCulling && bgfx::isValid(item->m_oqh))
			{
				// occlusionQuery
				for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
				{
					bgfx::submit(*view, progHandle, item->m_oqh, 0, it != itEnd - 1 && view != viewEnd - 1);
				}
			}
			else
			{
				for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
				{
					bgfx::submit(*view, progHandle, 0, it != itEnd - 1 && view != viewEnd - 1);
				}
			}
		}
	}

	/*virtual*/ 
	const AgRenderItem* AgRenderMultiNode::getItem(uint16_t id) const
	{
		if(id >= m_items.size())
			return nullptr;

		return m_items.at(id);
	}
}