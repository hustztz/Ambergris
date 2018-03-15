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

	/*virtual*/
	void AgRenderSingleNode::draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery) const
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

		if (pFxSystem && typeid(*pFxSystem) == typeid(AgHardwarePickingSystem))
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
			for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
			{
				bgfx::setCondition(m_item.m_oqh, true);//TODO
				bgfx::submit(*view, progHandle, 0, view != viewEnd - 1);
			}
		}
		else if (inOcclusionQuery && bgfx::isValid(m_item.m_oqh))
		{
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