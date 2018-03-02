#include "AgTexture.h"
#include "Foundation/AgFileUtils.h"

#include <bx/macros.h>
#include <bimg/decode.h>
#include "BGFX/entry/dbg.h" // TODO
#include "BGFX/entry/entry.h" // TODO

namespace ambergris {

	/*virtual*/
	void AgTextureManager::destroy()
	{
		for (int i = 0;i < getSize(); ++i)
		{
			AgTexture* tex = get(i);
			if(!tex)
				continue;
			if (bgfx::isValid(tex->m_tex_handle))
			{
				bgfx::destroy(tex->m_tex_handle);
			}
		}
		AgResourcePool<AgTexture>::destroy();
	}

	static void imageReleaseCb(void* _ptr, void* _userData)
	{
		BX_UNUSED(_ptr);
		bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
		bimg::imageFree(imageContainer);
	}

	AgTexture::Handle AgTextureManager::append(bx::AllocatorI* allocator, bx::FileReaderI* _reader, const char* _name, uint32_t _flags)
	{
		uint32_t size;
		void* data = fileUtils::load(_reader, allocator, _name, &size);
		if (NULL == data)
			return AgResource::kInvalidHandle;

		bimg::ImageContainer* imageContainer = bimg::imageParse(allocator, data, size);
		if (NULL == imageContainer)
			return AgResource::kInvalidHandle;

		if ((NULL != bx::strFindI(_name, ".dds")) && (NULL != bx::strFindI(_name, ".ktx")))
		{
			DBG("Convert texture: %s", _name);
			bimg::TextureFormat::Enum outputFormat = imageContainer->m_format;
			bimg::ImageContainer* output = bimg::imageConvert(allocator, outputFormat, *imageContainer);
			bimg::imageFree(imageContainer);
			imageContainer = output;
		}
		
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

		AgTexture* tex = AgResourcePool<AgTexture>::allocate<AgTexture>(allocator);
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

	AgTexture::Handle AgTextureManager::append(uint16_t _width
		, uint16_t _height
		, uint16_t _depth
		, bool     _hasMips
		, uint16_t _numLayers
		, bgfx::TextureFormat::Enum _format
		, uint32_t _flags)
	{
		bgfx::TextureHandle handle = BGFX_INVALID_HANDLE;
		if (0 == _depth && 0 == _height)
		{
			handle = bgfx::createTextureCube(_width, _hasMips, _numLayers, _format, _flags);
		}
		else if(0 == _depth)
		{
			handle = bgfx::createTexture2D(_width, _height, _hasMips, _numLayers, _format, _flags);
		}
		else
		{
			handle = bgfx::createTexture3D(_width, _height, _depth, _hasMips, _format, _flags);
		}

		if (bgfx::isValid(handle))
		{
			AgTexture* tex = AgResourcePool<AgTexture>::allocate<AgTexture>(entry::getAllocator());
			if (!tex)
				return AgResource::kInvalidHandle;
			tex->m_tex_handle = handle;
			return tex->m_handle;
		}
		
		return AgTexture::kInvalidHandle;
	}
}