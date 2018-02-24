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
		struct Uniform
		{
			enum UniformType
			{
				INT,
				FLOAT,
				MATRIX4,
				TIME,
				COLOR,
				VEC2,
				VEC3
			};

			UniformType type;
			bgfx::UniformHandle handle;
		};
		struct TextureSlot
		{
			TextureSlot()
				: texture_handle(AgResource::kInvalidHandle)
				, uniform_handle(BGFX_INVALID_HANDLE)
				, texture_state(0)
			{}

			AgResource::Handle	texture_handle;
			bgfx::UniformHandle uniform_handle;
			uint64_t            texture_state;
			//uint8_t             texture_stage;
		};
		static const int MAX_TEXTURE_SLOT_COUNT = 16;

		AgShader() : m_program(BGFX_INVALID_HANDLE), m_texture_slot_count(0) {}

		TextureSlot				m_texture_slots[MAX_TEXTURE_SLOT_COUNT];
		uint8_t					m_texture_slot_count;
		stl::vector<Uniform>	m_uniforms;
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

