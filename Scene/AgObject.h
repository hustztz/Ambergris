#pragma once

#include "Resource/AgResource.h"
#include "Resource/AgResourcePool.h"
#include "Resource/AgBoundingBox.h"

#include <tinystl/allocator.h>
#include <tinystl/string.h>
namespace stl = tinystl;


namespace ambergris {

	struct AgObject : public AgResource
	{
		AgObject() : AgResource(), m_parent_handle(AgResource::kInvalidHandle), m_dirty(true){
		}
		virtual ~AgObject() {}

		AgBoundingbox::Handle	m_bbox;
		stl::string		m_name;
		Handle			m_parent_handle;
		float			m_local_transform[16];
		float			m_global_transform[16];
		std::atomic<bool>	m_dirty;
	};

	class AgObjectManager : public AgResourcePool<AgObject>
	{
	public:
		AgObjectManager() : AgResourcePool<AgObject>(), m_dirty(false) {}

		std::atomic<bool>		m_dirty;
	};
}