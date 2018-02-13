#pragma once
#include "BgfxRenderNode.h"
#include "Foundation/TBuffer.h"

namespace ambergris_bgfx {

	class BgfxRenderInstanceNode : public BgfxRenderNode
	{
		static const uint16_t TRANSFORM_STRIDE = 64;
		static const uint16_t TRANSFORM_DATA_STRIDE = 64;
	public:
		BgfxRenderInstanceNode()
			: BgfxRenderNode()
			, m_stride(0)
		{
			m_instance_db.data = nullptr;
			m_instance_db.size = 0;
		}
		~BgfxRenderInstanceNode()
		{
		}

		virtual void DestroyGeometry() override;
		virtual void Draw(bgfx::ViewId view) const override;
		virtual bool AppendGeometry(
			const float* transform,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) override;

		bool Prepare();
		bool AppendInstance(const float* data, unsigned int size);
	private:
		ambergris::TBuffer<float>	m_instance_buffer;
		bgfx::InstanceDataBuffer	m_instance_db;
		uint16_t					m_stride;
	};
}