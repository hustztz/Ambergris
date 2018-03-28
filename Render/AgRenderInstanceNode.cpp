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
		AgMaterial::Handle material,
		AgBoundingbox::Handle bbox,
		const uint32_t* pick_id,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		bool ret = AgRenderSingleNode::appendGeometry(
			nullptr,
			material,
			AgBoundingbox::kInvalidHandle,
			nullptr,
			decl,
			vertBuf, vertSize,
			indexBuf, indexSize);
		if (!ret)
			return false;

		// need reprepare
		m_evaluated = false;

		if (!bgfx::isValid(m_instance_db))
			m_instance_db = bgfx::createDynamicVertexBuffer(1, ms_inst_decl, BGFX_BUFFER_ALLOW_RESIZE);

		static const int stride = 16;
		float instanceData[stride];
		ret &= (0 != memcpy_s(instanceData, stride * sizeof(float), transform, stride * sizeof(float)));
		instanceData[3] = pick_id[0] / 255.0f;
		instanceData[7] = pick_id[1] / 255.0f;
		instanceData[11] = pick_id[2] / 255.0f;

		return appendInstance(instanceData, stride);
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
		// need reprepare
		m_evaluated = false;
		return true;
	}

	/*virtual*/
	bool AgRenderInstanceNode::prepare() const
	{
		if (m_evaluated)
			return true;

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

		const bgfx::Memory* mem = bgfx::makeRef(m_instance_buffer.GetData(), m_instance_buffer.GetSize() * sizeof(float));
		bgfx::updateDynamicVertexBuffer(m_instance_db, 0, mem);
		return bgfx::isValid(m_instance_db);
	}

	/*virtual*/
	void AgRenderInstanceNode::draw(const ViewIdArray& views, const AgFxSystem* pFxSystem, int32_t occlusionCulling) const
	{
		if (!prepare())
			return;

		if (!bgfx::isValid(m_item.m_vbh) || !bgfx::isValid(m_item.m_ibh))
			return;

		const uint32_t numInstances = m_instance_buffer.GetSize() / m_stride;
		if (0 == numInstances)
			return;

		if (!pFxSystem || AgShader::E_COUNT == pFxSystem->getOverrideShader())
			return;

		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(pFxSystem->getOverrideShader() + AgShader::SHADER_INSTANCE_OFFSET);
		if (!shader || AgShader::kInvalidHandle == shader->m_handle)
			return;

		if (pFxSystem && typeid(*pFxSystem) == typeid(AgHardwarePickingSystem))
		{
			if (20 != m_stride)
			{
				printf("Instance node has no color buffer for picking.\n");
			}
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

		// Set instance data buffer.
		//prepare();
		//bgfx::setInstanceDataBuffer(&m_instance_db);
		bgfx::setInstanceDataBuffer(m_instance_db, 0, numInstances);

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
		for (ViewIdArray::const_iterator view = views.cbegin(), viewEnd = views.cend(); view != viewEnd; view++)
		{
			bgfx::submit(*view, shader->m_program, 0, view != viewEnd - 1);
		}
	}

	/*virtual*/
	const float* AgRenderInstanceNode::getTransform(uint16_t id) const
	{
		uint16_t transformPos = id * m_stride;
		if(transformPos + 16 > (uint16_t)m_instance_buffer.GetSize())
			return nullptr;

		return m_instance_buffer.GetData() + transformPos;
	}
}