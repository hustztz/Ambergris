#pragma once

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
		static const uint8_t SHADER_INSTANCE_OFFSET = 4;
		enum ShaderType
		{
			E_LAMBERT_SHADER = 0,
			E_INSTANCE_SHADER,
			E_PHONG_SHADER,
			E_PICKING_SHADER,

			E_LAMBERT_INSTANCE_SHADER,
			E_INSTANCE_INSTANCE_SHADER,
			E_PHONG_INSTANCE_SHADER,
			E_PICKING_INSTANCE_SHADER,

			E_COUNT
		};
		struct UniformSlot
		{
			UniformSlot()
				: uniform_handle(BGFX_INVALID_HANDLE)
			{}

			bgfx::UniformHandle uniform_handle;
		};
		struct TextureSlot : public UniformSlot
		{
			TextureSlot()
				: UniformSlot()
				, texture_state(0)
			{}
			uint32_t            texture_state;
		};

		AgShader() : m_program(BGFX_INVALID_HANDLE) {
		}

		uint8_t	getTextureSlotSize() const
		{
			uint8_t size = 0;
			for (uint8_t i = 0; i < MAX_TEXTURE_SLOT_COUNT; i++)
			{
				if (bgfx::isValid(m_texture_slots[i].uniform_handle))
					size++;
			}
			return size;
		}
		uint8_t	getUniformSize() const
		{
			uint8_t size = 0;
			for (uint8_t i = 0; i < MAX_UNIFORM_COUNT; i++)
			{
				if (bgfx::isValid(m_uniforms[i].uniform_handle))
					size++;
			}
			return size;
		}

		static const int MAX_TEXTURE_SLOT_COUNT = 8;
		TextureSlot				m_texture_slots[MAX_TEXTURE_SLOT_COUNT];
		static const int MAX_UNIFORM_COUNT = 16;
		UniformSlot				m_uniforms[MAX_UNIFORM_COUNT];
		bgfx::ProgramHandle		m_program;
	};

	class AgShaderManager : public AgResourceContainer<AgShader, AgShader::E_COUNT>
	{
	public:
		bool loadShader(bx::FileReaderI* _reader);
		void unloadShader();
	protected:
		friend class AgMaterialManager;
	};
}

