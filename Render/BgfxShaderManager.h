#pragma once

#include "Foundation/Singleton.h"

#include <bgfx/bgfx.h>

namespace bx {
	struct FileReaderI;
}

namespace ambergris_bgfx {

	class BgfxShaderManager
	{
	protected:
		bool loadShader(bx::FileReaderI* _reader);
		void unloadShader();
	protected:
		friend class BgfxMaterialManager;
	private:
		BgfxShaderManager();
		~BgfxShaderManager();
		friend class Singleton<BgfxShaderManager>;
		friend class BgfxRenderNode;
	private:
		bgfx::ProgramHandle m_mesh_program;
		bgfx::ProgramHandle m_instancing_program;
	};
}

