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

	AgTexture::Handle AgTextureManager::append(const char* _name, uint32_t _flags)
	{
		uint32_t size;
		void* data = fileUtils::load(_name, &size);
		if (NULL == data)
			return AgTexture::kInvalidHandle;

		bimg::ImageContainer* imageContainer = bimg::imageParse(entry::getAllocator(), data, size);
		if (NULL == imageContainer)
			return AgTexture::kInvalidHandle;

		if ((NULL != bx::strFindI(_name, ".dds")) && (NULL != bx::strFindI(_name, ".ktx")))
		{
			DBG("Convert texture: %s", _name);
			bimg::TextureFormat::Enum outputFormat = imageContainer->m_format;
			bimg::ImageContainer* output = bimg::imageConvert(entry::getAllocator(), outputFormat, *imageContainer);
			bimg::imageFree(imageContainer);
			imageContainer = output;
		}
		
		const bgfx::Memory* mem = bgfx::makeRef(
			imageContainer->m_data
			, imageContainer->m_size
			, imageReleaseCb
			, imageContainer
		);
		fileUtils::unload(data);

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
		if(!bgfx::isValid(handle))
			return AgTexture::kInvalidHandle;

		bgfx::setName(handle, _name);

		std::shared_ptr<AgTexture> tex(BX_NEW(entry::getAllocator(), AgTexture));
		if (!tex)
			return AgTexture::kInvalidHandle;
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

		return AgResourcePool<AgTexture>::append(tex);
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
			std::shared_ptr<AgTexture> tex(BX_NEW(entry::getAllocator(), AgTexture));
			if (!tex)
				return AgResource::kInvalidHandle;
			tex->m_tex_handle = handle;
			return AgResourcePool<AgTexture>::append(tex);
		}
		
		return AgTexture::kInvalidHandle;
	}

	AgTexture::Handle AgTextureManager::find(const char* _name)
	{
		for (int i = 0; i < getSize(); ++i)
		{
			const AgTexture* tex = get(i);
			if (!tex)
				continue;
			if (bgfx::isValid(tex->m_tex_handle))
			{
				if (0 == std::strcmp(tex->m_name.c_str(), _name))
				{
					return i;
				}
			}
		}
		return AgTexture::kInvalidHandle;
	}
}