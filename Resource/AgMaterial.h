#pragma once

#include "Foundation/Singleton.h"
#include "AgResourceContainer.h"

#include <bgfx/bgfx.h>
#include <tinystl/allocator.h>
#include <tinystl/string.h>
namespace stl = tinystl;

namespace bx {
	struct FileReaderI;
}

namespace ambergris {

	struct AgMaterial : public AgResource
	{
		enum MaterialType
		{
			E_LAMBERT = 0,
			E_PHONG,

			E_COUNT
		};

		AgMaterial() : m_state_flags(0), m_shader(AgResource::kInvalidHandle) {}

		stl::string		m_name;
		uint64_t        m_state_flags;
		Handle			m_shader;
	};

	class AgMaterialManager : public AgResourceContainer<AgMaterial, AgMaterial::E_COUNT>
	{
	public:
		void Init(bx::FileReaderI* _reader);
		virtual void Destroy() override;
		uint64_t GetRenderState(AgMaterial::MaterialType id) const;
		bgfx::ProgramHandle GetProgramHandle(AgMaterial::MaterialType id) const;
	protected:
	private:
		AgMaterialManager() {};
		~AgMaterialManager() {};
		friend class Singleton<AgMaterialManager>;
		friend class AgRenderNode;
	};
}