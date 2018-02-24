#pragma once

#include <bgfx/bgfx.h>
#include <bx/readerwriter.h>

namespace ambergris {
	namespace shaderUtils {
		bgfx::ProgramHandle loadProgram(bx::FileReaderI* _reader, const char* _vsName, const char* _fsName);
	}
}
