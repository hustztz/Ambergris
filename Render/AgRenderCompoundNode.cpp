#include "AgRenderCompoundNode.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgRenderItem.h"
#include "AgHardwarePickingSystem.h"

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
	void AgRenderCompoundNode::draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery)
	{
		if (m_items.empty())
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

		if (pFxSystem && typeid(*pFxSystem) != typeid(AgHardwarePickingSystem))
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
			if (pFxSystem && typeid(*pFxSystem) == typeid(AgHardwarePickingSystem))
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
			if (AgShader::E_SIMPLE_SHADER == pFxSystem->getOverrideShader() && bgfx::isValid(it->m_oqh))
			{
				for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
				{
					bgfx::setCondition(it->m_oqh, true);//TODO
					bgfx::submit(*view, progHandle, 0 , it != itEnd - 1 && view != viewEnd - 1);
				}
			}
			else if (inOcclusionQuery && bgfx::isValid(it->m_oqh))
			{
				for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
				{
					bgfx::submit(*view, progHandle, it->m_oqh, 0, it != itEnd - 1 && view != viewEnd - 1);
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

	/*virtual*/ const AgRenderItem* AgRenderCompoundNode::findItem(const uint32_t* pick_id) const
	{
		for (AgRenderCompoundNode::RenderItemArray::const_iterator it = m_items.cbegin(), itEnd = m_items.cend(); it != itEnd; ++it)
		{
			if (it->m_pick_id[0] == pick_id[0] &&
				it->m_pick_id[1] == pick_id[1] &&
				it->m_pick_id[2] == pick_id[2])
			{
				return &*it;
			}
		}
		return nullptr;
	}
}