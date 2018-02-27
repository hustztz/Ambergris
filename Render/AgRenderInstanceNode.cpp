#include "AgRenderInstanceNode.h"

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
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		assert(m_items.empty());
		if (!m_items.empty())
			return false;

		return AgRenderNode::appendGeometry(
			nullptr,
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
	void AgRenderInstanceNode::draw(bgfx::ViewId view) const
	{
		if (m_items.empty())
			return;
		const AgMaterial* mat = Singleton<AgMaterialManager>::instance().get(m_material_handle);
		if (!mat)
			return;
		uint64_t state = mat->m_state_flags;
		bgfx::ProgramHandle progHandle = mat->getProgramHandle();
		bgfx::setState(state);
		m_items[0].submitBuffers();
		// Set instance data buffer.
		bgfx::setInstanceDataBuffer(&m_instance_db);
		bgfx::submit(view, progHandle);
	}
}