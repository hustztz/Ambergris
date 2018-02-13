#pragma once

#include <bgfx/bgfx.h>
#include <bx/readerwriter.h>

namespace ambergris {
	namespace fileUtils {
		const bgfx::Memory* loadMem(bx::FileReaderI* _reader, const char* _filePath);
	}
}
