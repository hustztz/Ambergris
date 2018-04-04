#include "AgBoundingBox.h"

namespace ambergris {

	void AgBoundingbox::getCorners(float(*corners)[3]) const
	{
		if (AgBoundingbox::kInvalidHandle == m_handle)
			return;

		if (!corners)
			return;

		corners[0][0] = m_aabb.m_min[0]; corners[0][1] = m_aabb.m_min[1], corners[0][2] = m_aabb.m_min[2];
		corners[1][0] = m_aabb.m_max[0]; corners[1][1] = m_aabb.m_min[1], corners[1][2] = m_aabb.m_min[2];
		corners[2][0] = m_aabb.m_min[0]; corners[2][1] = m_aabb.m_max[1], corners[2][2] = m_aabb.m_min[2];
		corners[3][0] = m_aabb.m_max[0]; corners[3][1] = m_aabb.m_max[1], corners[3][2] = m_aabb.m_min[2];

		corners[4][0] = m_aabb.m_min[0]; corners[4][1] = m_aabb.m_min[1], corners[4][2] = m_aabb.m_max[2];
		corners[5][0] = m_aabb.m_max[0]; corners[5][1] = m_aabb.m_min[1], corners[5][2] = m_aabb.m_max[2];
		corners[6][0] = m_aabb.m_min[0]; corners[6][1] = m_aabb.m_max[1], corners[6][2] = m_aabb.m_max[2];
		corners[7][0] = m_aabb.m_max[0]; corners[7][1] = m_aabb.m_max[1], corners[7][2] = m_aabb.m_max[2];
	}

	float AgBoundingbox::getRadius() const
	{
		if (AgBoundingbox::kInvalidHandle == m_handle)
			return 0.0f;

		//return m_sphere.m_radius;
		return sqrt((m_aabb.m_max[0] - m_aabb.m_min[0])*(m_aabb.m_max[0] - m_aabb.m_min[0]) + (m_aabb.m_max[1] - m_aabb.m_min[1])*(m_aabb.m_max[1] - m_aabb.m_min[1]) + (m_aabb.m_max[2] - m_aabb.m_min[2])*(m_aabb.m_max[2] - m_aabb.m_min[2]));
	}

	void AgBoundingbox::expand(float val)
	{
		m_aabb.m_max[0] += val;
		m_aabb.m_max[1] += val;
		m_aabb.m_max[2] += val;
		m_aabb.m_min[0] += val;
		m_aabb.m_min[1] += val;
		m_aabb.m_min[2] += val;

		m_sphere.m_radius += val;
	}

	bool AgBoundingBoxManager::setBoundingBox(AgBoundingbox::Handle& id, const float* min, const float* max)
	{
		if (!min || !max)
			return false;

		AgBoundingbox* bounds = get(id);
		if (bounds)
		{
			bounds->m_aabb.m_min[0] = min[0];
			bounds->m_aabb.m_min[1] = min[1];
			bounds->m_aabb.m_min[2] = min[2];
			bounds->m_aabb.m_max[0] = max[0];
			bounds->m_aabb.m_max[1] = max[1];
			bounds->m_aabb.m_max[2] = max[2];
		}
		else
		{
			std::shared_ptr<AgBoundingbox> newBbox(new AgBoundingbox);
			newBbox->m_aabb.m_min[0] = min[0];
			newBbox->m_aabb.m_min[1] = min[1];
			newBbox->m_aabb.m_min[2] = min[2];
			newBbox->m_aabb.m_max[0] = max[0];
			newBbox->m_aabb.m_max[1] = max[1];
			newBbox->m_aabb.m_max[2] = max[2];
			id = append(newBbox);
		}

		m_dirty = true;
		return AgBoundingbox::kInvalidHandle != id;
	}
}