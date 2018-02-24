#pragma once

#include "Foundation/Singleton.h"
#include "AgResourceManager.h"

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

	class AgTextureManager : public AgResourceManager<AgTexture>
	{
	public:
		AgResource::Handle Append(bx::AllocatorI* allocator, bx::FileReaderI* _reader, const char* _name, uint32_t _flags);
		virtual void Destroy() override;
		bgfx::TextureHandle GetTextureHandle(AgResource::Handle handle) const;
	protected:
	private:
		AgTextureManager() {};
		~AgTextureManager() {};
		friend class Singleton<AgTextureManager>;
		friend class AgRenderNode;
	};
}