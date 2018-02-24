#pragma once

#include <bgfx/bgfx.h>

namespace ambergris {

	class AgRenderItem
	{
	public:
		AgRenderItem();
		AgRenderItem(
			const float* transform,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize);
		~AgRenderItem()
		{
		}

		void DestroyBuffers();
		void SubmitBuffers() const;
		const float* GetTransform() const { return m_mtx; }
	protected:
		void _SetTransform(const float* mtx);
		void _SetVertexBuffer(const bgfx::VertexDecl& decl, const uint8_t* buffer, uint32_t size);
		void _SetIndexBuffer(const uint16_t* buffer, uint32_t size);

	private:
		bgfx::VertexBufferHandle	m_vbh;
		bgfx::IndexBufferHandle		m_ibh;
		float						m_mtx[16];
	};
}
