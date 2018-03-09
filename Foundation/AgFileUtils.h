#pragma once

#include <bgfx/bgfx.h>

namespace ambergris {
	namespace fileUtils {
		const bgfx::Memory* loadMem(const char* _filePath);
		void* load(const char* _filePath, uint32_t* _size);
		void unload(void* _ptr);
	}
}
