#pragma once

#include <string>
#include <tinystl/allocator.h>
#include <tinystl/vector.h>
namespace stl = tinystl;

#include "BGFX/bounds.h"

namespace ambergris {

	struct BgfxNodeHeirarchy
	{
		BgfxNodeHeirarchy() : m_mesh_handle(-1), m_inst_handle(-1), m_bShadowCaster(false){};

		struct Boundingbox
		{
			Sphere m_sphere;
			Aabb m_aabb;
			Obb m_obb;
		};
		Boundingbox		m_bb;
		std::string		m_name;
		std::string		m_parent;
		float			m_local_transform[16];
		float			m_global_transform[16];
		int				m_mesh_handle;
		int				m_inst_handle;
		bool			m_bShadowCaster;		// Initial shadow preference (overridden by renderables section of .unit)

		struct SubMesh
		{
			SubMesh() : m_material_index(-1) {};
			int		m_material_index;
			stl::vector<int>	m_primitives;
		};
		stl::vector<SubMesh>	m_subMesh;

	};
}