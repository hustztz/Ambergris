#pragma once

#include "Foundation/Singleton.h"
#include "AgResourceContainer.h"

#include <bgfx/bgfx.h>
#include <tinystl/allocator.h>
#include <tinystl/vector.h>
namespace stl = tinystl;

namespace bx {
	struct FileReaderI;
}

namespace ambergris {

	struct AgShader : public AgResource
	{
		enum ShaderType
		{
			E_LAMBERT_SHADER = 0,
			E_INSTANCE_SHADER,
			E_PHONG_SHADER,

			E_COUNT
		};
		struct TextureSlot
		{
			TextureSlot()
				: uniform_handle(BGFX_INVALID_HANDLE)
				, texture_state(0)
			{}

			bgfx::UniformHandle uniform_handle;
			uint32_t            texture_state;
		};

		AgShader() : m_program(BGFX_INVALID_HANDLE), m_texture_slot_count(0), m_uniform_count(0) {
			for (uint8_t i = 0; i < MAX_UNIFORM_COUNT; i ++)
			{
				m_uniforms[i] = BGFX_INVALID_HANDLE;
			}
		}

		static const int MAX_TEXTURE_SLOT_COUNT = 8;
		TextureSlot				m_texture_slots[MAX_TEXTURE_SLOT_COUNT];
		uint8_t					m_texture_slot_count;
		static const int MAX_UNIFORM_COUNT = 16;
		bgfx::UniformHandle		m_uniforms[MAX_UNIFORM_COUNT];
		uint8_t					m_uniform_count;
		bgfx::ProgramHandle		m_program;
	};

	class AgShaderManager : public AgResourceContainer<AgShader, AgShader::E_COUNT>
	{
	protected:
		bool loadShader(bx::FileReaderI* _reader);
		void unloadShader();
	protected:
		friend class AgMaterialManager;
	private:
		AgShaderManager();
		~AgShaderManager();
		friend class Singleton<AgShaderManager>;
		friend class AgRenderNode;
	};
}

