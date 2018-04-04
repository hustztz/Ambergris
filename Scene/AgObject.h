#pragma once

#include "Resource/AgResource.h"
#include "Resource/AgResourcePool.h"
#include "Resource/AgBoundingBox.h"
#include "Resource/AgCacheTransform.h"

#include <tinystl/allocator.h>
#include <tinystl/string.h>
namespace stl = tinystl;


namespace ambergris {

	struct AgObject : public AgResource
	{
		AgObject() : AgResource(), m_parent_handle(AgResource::kInvalidHandle), m_dirty(true), m_isVisible(true){
		}
		virtual ~AgObject() {}

		//virtual void setGlobalPosition(double* val);
		bool isVisible() const { return m_isVisible; }

		AgBoundingbox::Handle	m_bbox;
		stl::string		m_name;
		Handle			m_parent_handle;
		AgCacheTransform::Handle	m_global_transform_h;
		bool            m_isVisible;
		std::atomic<bool>	m_dirty;
	};

	class AgObjectManager : public AgResourcePool<AgObject>
	{
	public:
		AgObjectManager() : AgResourcePool<AgObject>(), m_dirty(false) {}

		std::atomic<bool>		m_dirty;
	};
}