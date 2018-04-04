#pragma once
#include "AgResourcePool.h"
#include "BGFX/bounds.h"

namespace ambergris {

	struct AgBoundingbox : public AgResource
	{
		AgBoundingbox() {}

		void getCorners(float (*corners)[3]) const;
		float getRadius() const;
		void expand(float val);

		Sphere m_sphere;
		Aabb m_aabb;
		Obb m_obb;
	};

	class AgBoundingBoxManager : public AgResourcePool<AgBoundingbox>
	{
	public:
		AgBoundingBoxManager() : AgResourcePool<AgBoundingbox>(), m_dirty(false) {}

		bool setBoundingBox(AgBoundingbox::Handle& id, const float* min, const float* max);

		std::atomic<bool>		m_dirty;
	};
}