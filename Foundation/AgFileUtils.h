#pragma once

#include <bgfx/bgfx.h>
#include <bx/readerwriter.h>

namespace ambergris {
	namespace fileUtils {
		const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath);
		void* load(bx::FileReaderI* _reader, bx::AllocatorI* _allocator, const char* _filePath, uint32_t* _size);
		void unload(bx::AllocatorI* _allocator, void* _ptr);
	}
}
