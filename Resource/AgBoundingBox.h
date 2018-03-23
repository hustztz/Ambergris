#pragma once
#include "AgResource.h"
#include "AgResourcePool.h"
#include "BGFX/bounds.h"

namespace ambergris {

	struct AgBoundingbox : public AgResource
	{
		AgBoundingbox() {}

		Sphere m_sphere;
		Aabb m_aabb;
		Obb m_obb;
	};

	class AgBoundingBoxManager : public AgResourcePool<AgBoundingbox>
	{
	public:
		AgBoundingBoxManager() : AgResourcePool<AgBoundingbox>(), m_dirty(false) {}

		std::atomic<bool>		m_dirty;
	};
}