#pragma once

#include "Resource/AgResource.h"
#include "BGFX/bounds.h"

#include <tinystl/allocator.h>
#include <tinystl/string.h>
namespace stl = tinystl;


namespace ambergris {

	struct AgObject : public AgResource
	{
		AgObject() : AgResource(), m_parent_handle(AgResource::kInvalidHandle){
		}
		virtual ~AgObject() {}

		struct Boundingbox
		{
			Boundingbox() : m_initialized(false) {}

			Sphere m_sphere;
			Aabb m_aabb;
			Obb m_obb;
			std::atomic<bool>	m_initialized;
		};
		Boundingbox		m_bb;
		stl::string		m_name;
		Handle			m_parent_handle;
		float			m_local_transform[16];
		float			m_global_transform[16];
	};
}