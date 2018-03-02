#include "AgRenderInstanceNode.h"
#include "Resource/AgRenderResourceManager.h"
#include "AgRenderPass.h"

#include <assert.h>

namespace ambergris {

	/*virtual*/
	void AgRenderInstanceNode::destroy()
	{
		AgRenderNode::destroy();
	}

	/*virtual*/
	bool AgRenderInstanceNode::appendGeometry(
		const float* transform,
		const uint32_t* pick_id,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		assert(m_items.empty());
		if (!m_items.empty())
			return false;

		return AgRenderNode::appendGeometry(
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

	bool AgRenderInstanceNode::prepare()
	{
		if (0 == m_stride)
			return false;

		assert(0 == m_instance_buffer.GetSize() % m_stride);
		const uint32_t numInstances = m_instance_buffer.GetSize() / m_stride;
		if (0 == numInstances)
			return false;

		if (numInstances != bgfx::getAvailInstanceDataBuffer(numInstances, m_stride))
			return false;

		bgfx::allocInstanceDataBuffer(&m_instance_db, numInstances, m_stride);
		return (0 == memcpy_s(m_instance_db.data, numInstances * m_stride, m_instance_buffer.GetData(), m_instance_buffer.GetSize()));
	}

	/*virtual*/
	void AgRenderInstanceNode::draw(bgfx::ViewId view,
		AgShader::Handle shaderHandle,
		uint64_t state,
		bool inOcclusionQuery /*= false*/,
		bool needOcclusionCondition /*= false*/)
	{
		if (m_items.empty())
			return;

		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(shaderHandle + AgShader::SHADER_INSTANCE_OFFSET);
		if (!shader)
			return;

		_SubmitTexture(shader);

		// Update Uniforms
		if (AgRenderPass::E_PASS_ID == view)
		{
			// Submit ID pass based on mesh ID
			float idsF[4];
			idsF[0] = m_items[0].m_pick_id[0] / 255.0f;
			idsF[1] = m_items[0].m_pick_id[1] / 255.0f;
			idsF[2] = m_items[0].m_pick_id[2] / 255.0f;
			idsF[3] = 1.0f;
			bgfx::setUniform(shader->m_uniforms[0].uniform_handle, idsF);
		}
		else
		{
			for (uint8_t i = 0; i < AgShader::MAX_UNIFORM_COUNT; ++i)
			{
				if (!bgfx::isValid(shader->m_uniforms[i].uniform_handle) || !m_items[0].m_uniformData[i].dirty)
					continue;
				bgfx::setUniform(shader->m_uniforms[i].uniform_handle, m_items[0].m_uniformData[i].data);
				m_items[0].m_uniformData[i].dirty = false;
			}
		}
		
		bgfx::setState(state);
		m_items[0].submit();
		// Set instance data buffer.
		bgfx::setInstanceDataBuffer(&m_instance_db);
		bgfx::submit(view, shader->m_program);
	}

}