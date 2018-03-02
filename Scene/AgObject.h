#pragma once

#include "Resource/AgResource.h"
#include "BGFX/bounds.h"

#include <bx/rng.h>

#include <tinystl/allocator.h>
#include <tinystl/string.h>
namespace stl = tinystl;


namespace ambergris {

	struct AgObject : public AgResource
	{
		AgObject() : AgResource(), m_parent_handle(AgResource::kInvalidHandle) {
			bx::RngMwc mwc;  // Random number generator
			m_pick_id[0] = mwc.gen() % 256;
			m_pick_id[1] = mwc.gen() % 256;
			m_pick_id[2] = mwc.gen() % 256;
		}
		virtual ~AgObject() {}

		struct Boundingbox
		{
			Sphere m_sphere;
			Aabb m_aabb;
			Obb m_obb;
		};
		Boundingbox		m_bb;
		stl::string		m_name;
		Handle			m_parent_handle;
		uint32_t		m_pick_id[3];
		float			m_local_transform[16];
		float			m_global_transform[16];
	};
}