#pragma once
#include "AgRenderSingleNode.h"
#include "Foundation/TBuffer.h"

namespace ambergris {

	class AgRenderInstanceNode : public AgRenderSingleNode
	{
		static const uint16_t TRANSFORM_STRIDE = 64;
		static const uint16_t TRANSFORM_DATA_STRIDE = 64;
	public:
		AgRenderInstanceNode()
			: AgRenderSingleNode()
			, m_stride(0)
			, m_instance_db(BGFX_INVALID_HANDLE)
		{
			if (0 == ms_inst_decl.getStride())
			{
				initDecl();
			}
		}
		~AgRenderInstanceNode()
		{
			destroy();
		}

		virtual void destroy() override;
		virtual void draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery) override;
		virtual bool appendGeometry(
			const float* transform,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) override;
		virtual bool prepare() override;

		bool appendInstance(const float* data, unsigned int size);
	private:
		TBuffer<float>				m_instance_buffer;
		//bgfx::InstanceDataBuffer	m_instance_db;
		bgfx::DynamicVertexBufferHandle m_instance_db;
		uint16_t					m_stride;
		static void initDecl()
		{
			ms_inst_decl
				.begin()
				.add(bgfx::Attrib::TexCoord4, 4, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord5, 4, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord6, 4, bgfx::AttribType::Float)
				.add(bgfx::Attrib::TexCoord7, 4, bgfx::AttribType::Float)
				.end();
		}
		static bgfx::VertexDecl		ms_inst_decl;
	};
}