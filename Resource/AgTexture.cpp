#include "AgTexture.h"
#include "Foundation/AgFileUtils.h"

#include <bx/macros.h>
#include <bimg/decode.h>

namespace ambergris {

	/*virtual*/
	void AgTextureManager::Destroy()
	{
		for (int i = 0;i < GetSize(); ++i)
		{
			AgTexture* tex = Get(i);
			if(!tex)
				continue;
			if (bgfx::isValid(tex->m_tex_handle))
			{
				bgfx::destroy(tex->m_tex_handle);
			}
		}
		AgResourceManager<AgTexture>::Destroy();
	}

	static void imageReleaseCb(void* _ptr, void* _userData)
	{
		BX_UNUSED(_ptr);
		bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
		bimg::imageFree(imageContainer);
	}

	AgResource::Handle AgTextureManager::Append(bx::AllocatorI* allocator, bx::FileReaderI* _reader, const char* _name, uint32_t _flags)
	{
		uint32_t size;
		void* data = fileUtils::load(_reader, allocator, _name, &size);
		if (NULL == data)
			return AgResource::kInvalidHandle;

		bimg::ImageContainer* imageContainer = bimg::imageParse(allocator, data, size);
		if (NULL == imageContainer)
			return AgResource::kInvalidHandle;
		
		const bgfx::Memory* mem = bgfx::makeRef(
			imageContainer->m_data
			, imageContainer->m_size
			, imageReleaseCb
			, imageContainer
		);
		fileUtils::unload(allocator, data);

		bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
		if (imageContainer->m_cubeMap)
		{
			handle = bgfx::createTextureCube(
				uint16_t(imageContainer->m_width)
				, 1 < imageContainer->m_numMips
				, imageContainer->m_numLayers
				, bgfx::TextureFormat::Enum(imageContainer->m_format)
				, _flags
				, mem
			);
		}
		else if (1 < imageContainer->m_depth)
		{
			handle = bgfx::createTexture3D(
				uint16_t(imageContainer->m_width)
				, uint16_t(imageContainer->m_height)
				, uint16_t(imageContainer->m_depth)
				, 1 < imageContainer->m_numMips
				, bgfx::TextureFormat::Enum(imageContainer->m_format)
				, _flags
				, mem
			);
		}
		else
		{
			handle = bgfx::createTexture2D(
				uint16_t(imageContainer->m_width)
				, uint16_t(imageContainer->m_height)
				, 1 < imageContainer->m_numMips
				, imageContainer->m_numLayers
				, bgfx::TextureFormat::Enum(imageContainer->m_format)
				, _flags
				, mem
			);
		}

		bgfx::setName(handle, _name);

		AgTexture* tex = AgResourceManager<AgTexture>::allocate<AgTexture>(allocator);
		if (!tex)
			return AgResource::kInvalidHandle;
		tex->m_name = stl::string(_name);
		bgfx::calcTextureSize(
			tex->m_info
			, uint16_t(imageContainer->m_width)
			, uint16_t(imageContainer->m_height)
			, uint16_t(imageContainer->m_depth)
			, imageContainer->m_cubeMap
			, 1 < imageContainer->m_numMips
			, imageContainer->m_numLayers
			, bgfx::TextureFormat::Enum(imageContainer->m_format)
		);
		tex->m_tex_handle = handle;

		return tex->m_handle;
	}

	bgfx::TextureHandle AgTextureManager::GetTextureHandle(AgResource::Handle handle) const
	{
		const AgTexture* tex = Get(handle);
		if (!tex)
			return BGFX_INVALID_HANDLE;
		return tex->m_tex_handle;
	}
}