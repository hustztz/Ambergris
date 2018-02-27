#pragma once

#include "Foundation/Singleton.h"
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
		AgResource::Handle append(bx::AllocatorI* allocator, bx::FileReaderI* _reader, const char* _name, uint32_t _flags);
		virtual void destroy() override;
	protected:
	private:
		AgTextureManager() {};
		~AgTextureManager() {};
		friend class Singleton<AgTextureManager>;
		friend class AgRenderNode;
	};
}