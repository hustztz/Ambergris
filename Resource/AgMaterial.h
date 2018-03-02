#pragma once

#include "AgResourceContainer.h"
#include "AgShader.h"

#include <bgfx/bgfx.h>
#include <tinystl/allocator.h>
#include <tinystl/string.h>
namespace stl = tinystl;

namespace ambergris {

	struct AgMaterial : public AgResource
	{
		enum MaterialType
		{
			E_LAMBERT = 0,
			E_PHONG,

			E_COUNT
		};

		AgMaterial() : m_state_flags(BGFX_STATE_DEFAULT), m_shader(AgResource::kInvalidHandle) {}

		AgShader::Handle getShaderHandle() const { return m_shader; }

		stl::string		m_name;
		uint64_t        m_state_flags;
		Handle			m_shader;
	};

	class AgMaterialManager : public AgResourceContainer<AgMaterial, AgMaterial::E_COUNT>
	{
	public:
		void init();
		virtual void destroy() override;
	protected:
	private:
		friend class AgRenderNode;
	};
}