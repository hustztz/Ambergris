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

		union Ambient
		{
			struct
			{
				float m_r;
				float m_g;
				float m_b;
				float m_unused;
			};

			float m_v[4];
		};

		union Diffuse
		{
			struct
			{
				float m_r;
				float m_g;
				float m_b;
				float m_unused;
			};

			float m_v[4];
		};

		union Specular
		{
			struct
			{
				float m_r;
				float m_g;
				float m_b;
				float m_ns;
			};

			float m_v[4];
		};

		AgMaterial();

		Ambient			m_ka;
		Diffuse			m_kd;
		Specular		m_ks;
		stl::string		m_name;
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