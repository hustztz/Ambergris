#include "AgRenderSingleNode.h"
#include "Resource/AgRenderResourceManager.h"

#include "AgHardwarePickingSystem.h"

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
		return true;
	}

	/*virtual*/
	void AgRenderSingleNode::draw(const ViewIdArray& views, AgFxSystem* pFxSystem, int32_t occlusionCulling) const
	{
		if (!bgfx::isValid(m_item.m_vbh) || !bgfx::isValid(m_item.m_ibh))
			return;

		const AgShader* shader = nullptr;
		uint64_t shaderState = BGFX_STATE_DEFAULT;
		if (pFxSystem && AgShader::E_COUNT != pFxSystem->getOverrideShader())
		{
			shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(pFxSystem->getOverrideShader());
			if(pFxSystem->getOverrideStates())
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

		if (pFxSystem && typeid(*pFxSystem) == typeid(AgHardwarePickingSystem))
		{
			// Submit ID pass based on mesh ID
			float idsF[4];
			idsF[0] = m_item.m_pick_id[0] / 255.0f;
			idsF[1] = m_item.m_pick_id[1] / 255.0f;
			idsF[2] = m_item.m_pick_id[2] / 255.0f;
			idsF[3] = 1.0f;
			pFxSystem->setPerDrawUniforms(shader, idsF);
		}
		else
		{
			if (!pFxSystem || pFxSystem->needTexture())
				_SubmitTexture(shader);

			if (pFxSystem)
			{
				pFxSystem->setPerDrawUniforms(shader, nullptr);
			}
			_SubmitUniform(shader, &m_item);
		}

		bgfx::setState(shaderState);
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