#pragma once

#include "Foundation/Singleton.h"

#include <string>

#include <bgfx/bgfx.h>

namespace bx {
	struct FileReaderI;
}

namespace ambergris_bgfx {

	class BgfxMaterialManager
	{
	public:
		enum MaterialIndex
		{
			E_LAMBERT = 0,
			E_PHONG,

			E_COUNT
		};
	private:
		struct BgfxMaterial
		{
			BgfxMaterial() : m_state_flags(0), m_program(BGFX_INVALID_HANDLE) {}

			std::string			m_name;
			uint64_t            m_state_flags;
			bgfx::ProgramHandle m_program;
		};
	public:
		void Init(bx::FileReaderI* _reader);
		void Destroy();
		uint64_t GetRenderState(MaterialIndex id) const;
		bgfx::ProgramHandle GetProgramHandle(MaterialIndex id) const;
	protected:
	private:
		BgfxMaterialManager() {};
		~BgfxMaterialManager() {};
		friend class Singleton<BgfxMaterialManager>;
		friend class BgfxRenderNode;
	private:
		BgfxMaterial m_mat[E_COUNT];
	};
}