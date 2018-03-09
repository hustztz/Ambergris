#include "AgFileUtils.h"
#include <bx/readerwriter.h>
#include "BGFX/entry/dbg.h"

#include "BGFX/entry/entry.h"//TODO

namespace ambergris {
	namespace fileUtils {

		const bgfx::Memory* loadMem(const char* _filePath)
		{
			bx::FileReaderI* _reader = entry::getFileReader();
			if (bx::open(_reader, _filePath))
			{
				uint32_t size = (uint32_t)bx::getSize(_reader);
				const bgfx::Memory* mem = bgfx::alloc(size + 1);
				bx::read(_reader, mem->data, size);
				bx::close(_reader);
				mem->data[mem->size - 1] = '\0';
				return mem;
			}

			DBG("Failed to load %s.", _filePath);
			return NULL;
		}

		void* load(const char* _filePath, uint32_t* _size)
		{
			bx::AllocatorI* _allocator = entry::getAllocator();
			bx::FileReaderI* _reader = entry::getFileReader();
			if (bx::open(_reader, _filePath))
			{
				uint32_t size = (uint32_t)bx::getSize(_reader);
				void* data = BX_ALLOC(_allocator, size);
				bx::read(_reader, data, size);
				bx::close(_reader);
				if (NULL != _size)
				{
					*_size = size;
				}
				return data;
			}
			else
			{
				DBG("Failed to open: %s.", _filePath);
			}

			if (NULL != _size)
			{
				*_size = 0;
			}

			return NULL;
		}

		void unload(void* _ptr)
		{
			bx::AllocatorI* _allocator = entry::getAllocator();
			BX_FREE(_allocator, _ptr);
		}
	}
}