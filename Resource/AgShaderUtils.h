#pragma once

#include <bgfx/bgfx.h>

namespace ambergris {
	namespace shaderUtils {
		bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName);
	}
}
