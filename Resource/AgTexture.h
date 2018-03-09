#pragma once

#include "AgResourcePool.h"

#include <bgfx/bgfx.h>
#include <tinystl/allocator.h>
#include <tinystl/string.h>
namespace stl = tinystl;

namespace bx {
	struct FileReaderI;
	struct AllocatorI;
}

namespace ambergris {

	struct AgTexture : public AgResource
	{
		AgTexture() : m_tex_handle(BGFX_INVALID_HANDLE) { m_info.storageSize = 0; }

		stl::string			m_name;
		bgfx::TextureInfo	m_info;
		bgfx::TextureHandle m_tex_handle;
	};

	class AgTextureManager : public AgResourcePool<AgTexture>
	{
	public:
		AgTexture::Handle append(const char* _name, uint32_t _flags);
		AgTexture::Handle append(uint16_t _width
			, uint16_t _height
			, uint16_t _depth
			, bool     _hasMips
			, uint16_t _numLayers
			, bgfx::TextureFormat::Enum _format
			, uint32_t _flags);
		AgTexture::Handle find(const char* _name);
		virtual void destroy() override;
	protected:
	private:
		friend class AgRenderNode;
	};
}