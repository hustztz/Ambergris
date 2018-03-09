#include "AgShaderUtils.h"
#include "Foundation/AgFileUtils.h"

#include <bx/bx.h>
#include <bx/string.h>

namespace ambergris {
	namespace shaderUtils {

		bgfx::ShaderHandle loadShader(const char* _name)
		{
			char filePath[512];

			const char* shaderPath = "???";

			switch (bgfx::getRendererType())
			{
			case bgfx::RendererType::Noop:
			case bgfx::RendererType::Direct3D9:  shaderPath = "shaders/dx9/";   break;
			case bgfx::RendererType::Direct3D11:
			case bgfx::RendererType::Direct3D12: shaderPath = "shaders/dx11/";  break;
			case bgfx::RendererType::Gnm:        shaderPath = "shaders/pssl/";  break;
			case bgfx::RendererType::Metal:      shaderPath = "shaders/metal/"; break;
			case bgfx::RendererType::OpenGL:     shaderPath = "shaders/glsl/";  break;
			case bgfx::RendererType::OpenGLES:   shaderPath = "shaders/essl/";  break;
			case bgfx::RendererType::Vulkan:     shaderPath = "shaders/spirv/"; break;

			case bgfx::RendererType::Count:
				BX_CHECK(false, "You should not be here!");
				break;
			}

			bx::strCopy(filePath, BX_COUNTOF(filePath), shaderPath);
			bx::strCat(filePath, BX_COUNTOF(filePath), _name);
			bx::strCat(filePath, BX_COUNTOF(filePath), ".bin");

			bgfx::ShaderHandle handle = bgfx::createShader(fileUtils::loadMem(filePath));
			bgfx::setName(handle, filePath);

			return handle;
		}

		bgfx::ProgramHandle loadProgram(const char* _vsName, const char* _fsName)
		{
			bgfx::ShaderHandle vsh = loadShader(_vsName);
			bgfx::ShaderHandle fsh = BGFX_INVALID_HANDLE;
			if (NULL != _fsName)
			{
				fsh = loadShader(_fsName);
			}

			return bgfx::createProgram(vsh, fsh, true /* destroy shaders when program is destroyed */);
		}
	}
}