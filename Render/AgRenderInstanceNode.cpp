#include "AgRenderInstanceNode.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgHardwarePickingSystem.h"

#include <assert.h>

namespace ambergris {

	bgfx::VertexDecl		AgRenderInstanceNode::ms_inst_decl;

	/*virtual*/
	void AgRenderInstanceNode::destroy()
	{
		AgRenderNode::destroy();
		if (bgfx::isValid(m_instance_db))
		{
			bgfx::destroy(m_instance_db);
			m_instance_db = BGFX_INVALID_HANDLE;
		}
	}

	/*virtual*/
	bool AgRenderInstanceNode::appendGeometry(
		const float* transform,
		const uint32_t* pick_id,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		return AgRenderSingleNode::appendGeometry(
			nullptr,
			nullptr,//TODO
			decl,
			vertBuf, vertSize,
			indexBuf, indexSize);
	}

	bool AgRenderInstanceNode::appendInstance(const float* data, unsigned int size)
	{
		if (0 == m_stride)
		{
			m_stride = (uint16_t)size;
		}
		else
		{
			assert((uint16_t)size == m_stride);
			if ((uint16_t)size != m_stride)
				return false;
		}
		m_instance_buffer.Append(data, size);
		return true;
	}

	/*virtual*/
	bool AgRenderInstanceNode::prepare()
	{
		if (0 == m_stride)
			return false;

		assert(0 == m_instance_buffer.GetSize() % m_stride);
		const uint32_t numInstances = m_instance_buffer.GetSize() / m_stride;
		if (0 == numInstances)
			return false;

		/*if (numInstances != bgfx::getAvailInstanceDataBuffer(numInstances, m_stride * sizeof(float)))
			return false;

		bgfx::allocInstanceDataBuffer(&m_instance_db, numInstances, m_stride * sizeof(float));
		return (0 == memcpy_s(m_instance_db.data, numInstances * m_stride * sizeof(float), m_instance_buffer.GetData(), m_instance_buffer.GetSize() * sizeof(float)));
		float* data = (float*)m_instance_db.data;
		for (int i = 0; i < numInstances * m_stride; i++)
		{
			*data = *(m_instance_buffer.GetData() + i);
			data = data + 1;
		}
		return true;*/

		if (!bgfx::isValid(m_instance_db))
			m_instance_db = bgfx::createDynamicVertexBuffer(numInstances, ms_inst_decl);

		const bgfx::Memory* mem = bgfx::makeRef(m_instance_buffer.GetData(), m_instance_buffer.GetSize() * sizeof(float));
		bgfx::updateDynamicVertexBuffer(m_instance_db, 0, mem);
		return bgfx::isValid(m_instance_db);
	}

	/*virtual*/
	void AgRenderInstanceNode::draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery)
	{
		if (!bgfx::isValid(m_item.m_vbh) || !bgfx::isValid(m_item.m_ibh))
			return;

		const uint32_t numInstances = m_instance_buffer.GetSize() / m_stride;
		if (0 == numInstances)
			return;

		const AgShader* shader = nullptr;
		uint64_t shaderState = BGFX_STATE_DEFAULT;
		if (pFxSystem && AgShader::E_COUNT != pFxSystem->getOverrideShader())
		{
			shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(pFxSystem->getOverrideShader() + AgShader::SHADER_INSTANCE_OFFSET);
			shaderState = pFxSystem->getOverrideStates();
		}
		else
		{
			const AgMaterial* mat = Singleton<AgRenderResourceManager>::instance().m_materials.get(m_material_handle);
			if (!mat)
				return;

			shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(mat->getShaderHandle() + AgShader::SHADER_INSTANCE_OFFSET);
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
		// Set instance data buffer.
		//prepare();
		//bgfx::setInstanceDataBuffer(&m_instance_db);
		bgfx::setInstanceDataBuffer(m_instance_db, 0, numInstances);
		for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
		{
			bgfx::submit(*view, shader->m_program, 0, view != viewEnd - 1);
		}
	}

}