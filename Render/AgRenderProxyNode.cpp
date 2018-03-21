#include "AgRenderProxyNode.h"
#include "AgRenderItem.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgFxSystem.h"

#include <assert.h>

namespace ambergris {

	/*virtual*/
	bool AgRenderProxyNode::appendGeometry(
		const float* transform,
		AgMaterial::Handle material,
		const uint32_t* pick_id,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		return false;
	}

	/*virtual*/
	void AgRenderProxyNode::draw(const ViewIdArray& views, AgFxSystem* pFxSystem, int32_t occlusionCulling) const
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
			if (pFxSystem->getOverrideStates())
				shaderState = pFxSystem->getOverrideStates();
			else
				shaderState = m_renderState;
		}
		else
		{
			shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(m_shader);
			shaderState = m_renderState;
		}
		
		if (!shader || !shader->m_prepared)
			return;

		if (!pFxSystem || pFxSystem->needTexture())
			_SubmitTexture(shader);

		if (pFxSystem)
		{
			pFxSystem->setPerDrawUniforms(shader, nullptr);
		}
		_SubmitUniform(shader, m_pItem);

		bgfx::setState(shaderState);
		m_pItem->submit();
		// Override transform
		bgfx::setTransform(m_mtx);

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

	void AgRenderProxyNode::setTransform(const float* mtx)
	{
		if (mtx)
		{
			memcpy_s(m_mtx, 16 * sizeof(float), mtx, 16 * sizeof(float));
			m_mtx[3] = m_mtx[7] = m_mtx[11] = 0.0f;
		}
		else
		{
			m_mtx[0] = m_mtx[5] = m_mtx[9] = m_mtx[15] = 1.0f;
			m_mtx[1] = m_mtx[2] = m_mtx[3] = m_mtx[4] =
				m_mtx[6] = m_mtx[7] = m_mtx[8] = m_mtx[10] =
				m_mtx[11] = m_mtx[12] = m_mtx[13] = m_mtx[14] = 0.0f;
		}
	}
}