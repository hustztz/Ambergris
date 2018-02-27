#pragma once
#include "AgRenderNode.h"
#include "Foundation/TBuffer.h"

namespace ambergris {

	class AgRenderInstanceNode : public AgRenderNode
	{
		static const uint16_t TRANSFORM_STRIDE = 64;
		static const uint16_t TRANSFORM_DATA_STRIDE = 64;
	public:
		AgRenderInstanceNode()
			: AgRenderNode()
			, m_stride(0)
		{
			m_instance_db.data = nullptr;
			m_instance_db.size = 0;
		}
		~AgRenderInstanceNode()
		{
		}

		virtual void destroy() override;
		virtual void draw(bgfx::ViewId view) const override;
		virtual bool appendGeometry(
			const float* transform,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) override;

		bool prepare();
		bool appendInstance(const float* data, unsigned int size);
	private:
		TBuffer<float>				m_instance_buffer;
		bgfx::InstanceDataBuffer	m_instance_db;
		uint16_t					m_stride;
	};
}