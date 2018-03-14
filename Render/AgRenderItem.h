#pragma once

#include "Resource/AgTexture.h"
#include "Resource/AgShader.h"

#include <atomic>

namespace ambergris {

	class AgRenderItem
	{
	public:
		struct UniformData
		{
			UniformData() : data(nullptr)/*, dirty(false)*/ {}
			UniformData& operator= (const UniformData& other) {
				this->type = other.type;
				this->data = other.data;
				//this->dirty = false;//atomic has no operator= function.
				return *this;
			}
			UniformData(const UniformData& other) {
				this->type = other.type;
				this->data = other.data;
				//this->dirty = false;//atomic has no copy construction function.
			}
			bgfx::UniformType::Enum type;
			void*					data;
			//std::atomic<bool>		dirty;
		};
	public:
		AgRenderItem();
		~AgRenderItem();

		void setBuffers(
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize);
		void setTransform(const float* mtx);
		void setPickID(const uint32_t* id);

		void destroyBuffers();
		void submit() const;

		void enableOcclusionQuery();
		void disableOcclusionQuery();
	protected:
		void _ResetTransform();
		void _SetVertexBuffer(const bgfx::VertexDecl& decl, const uint8_t* buffer, uint32_t size);
		void _SetIndexBuffer(const uint16_t* buffer, uint32_t size);

	private:
		friend class AgRenderNode;
		friend class AgRenderSingleNode;
		friend class AgRenderInstanceNode;
		friend class AgRenderCompoundNode;
		friend class AgRenderProxyNode;

		bgfx::OcclusionQueryHandle	m_oqh;
		bgfx::VertexBufferHandle	m_vbh;
		bgfx::IndexBufferHandle		m_ibh;
		float						m_mtx[16];
		uint32_t					m_pick_id[3];
		UniformData					m_uniformData[AgShader::MAX_UNIFORM_COUNT];
	};
}
