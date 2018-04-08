#pragma once
#include "AgResourcePool.h"
#include "BGFX/bounds.h"
#include <common/RCBox.h>

namespace ambergris {

	struct AgBoundingbox : public AgResource
	{
		AgBoundingbox() {}

		RealityComputing::Common::RCBox           m_bounds;

		/*void getCorners(float (*corners)[3]) const;
		float getRadius() const;
		void expand(float val);

		Sphere m_sphere;
		Aabb m_aabb;
		Obb m_obb;*/
	};

	class AgBoundingBoxManager : public AgResourcePool<AgBoundingbox>
	{
	public:
		AgBoundingBoxManager() : AgResourcePool<AgBoundingbox>(), m_dirty(false) {}

		void setBoundingBox(AgBoundingbox::Handle& id, const RealityComputing::Common::RCBox& bounds);

		std::atomic<bool>		m_dirty;
	};
}