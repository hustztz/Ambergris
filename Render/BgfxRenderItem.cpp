#include "BgfxRenderItem.h"

#include <memory.h>

namespace ambergris_bgfx {

	BgfxRenderItem::BgfxRenderItem()
	{
	}

	BgfxRenderItem::BgfxRenderItem(
		const float* transform,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		_SetTransform(transform);
		_SetVertexBuffer(decl, vertBuf, vertSize);
		_SetIndexBuffer(indexBuf, indexSize);
	}

	void BgfxRenderItem::DestroyBuffers()
	{
		if (bgfx::isValid(m_vbh))
		{
			//bgfx::destroy(m_vbh);
			m_vbh.idx = bgfx::kInvalidHandle;
		}
		if (bgfx::isValid(m_ibh))
		{
			//bgfx::destroy(m_ibh);
			m_ibh.idx = bgfx::kInvalidHandle;
		}
	}

	void BgfxRenderItem::_SetVertexBuffer(const bgfx::VertexDecl& decl, const uint8_t* buffer, uint32_t size)
	{
		if (!buffer || 0 == size)
			return;
		const bgfx::Memory* mem = bgfx::alloc(size);
		m_vbh = bgfx::createVertexBuffer(mem, decl);
		memcpy_s(mem->data, mem->size, buffer, size);
	}

	void BgfxRenderItem::_SetIndexBuffer(const uint16_t* buffer, uint32_t size)
	{
		if (!buffer || 0 == size)
			return;
		const bgfx::Memory* mem = bgfx::alloc(size);
		m_ibh = bgfx::createIndexBuffer(mem);
		memcpy_s(mem->data, mem->size, buffer, size);
	}

	void BgfxRenderItem::SubmitBuffers() const
	{
		bgfx::setTransform(m_mtx);
		bgfx::setIndexBuffer(m_ibh);
		bgfx::setVertexBuffer(0, m_vbh);
	}

	void BgfxRenderItem::_SetTransform(const float* mtx)
	{
		if(mtx)
			memcpy_s(m_mtx, 16 * sizeof(float), mtx, 16 * sizeof(float));
		else
		{
			m_mtx[0] = m_mtx[5] = m_mtx[9] = m_mtx[15] = 1.0f;
			m_mtx[1] = m_mtx[2] = m_mtx[3] = m_mtx[4] =
				m_mtx[6] = m_mtx[7] = m_mtx[8] = m_mtx[10] = 
				m_mtx[11] = m_mtx[12] = m_mtx[13] = m_mtx[14] = 0.0f;
		}
	}

}
