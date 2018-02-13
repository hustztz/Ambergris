#include "SmtBoundingbox.h"

#include <limits>

void SmtBoundingbox::reset()
{
	m_xmin = std::numeric_limits<double>::max();
	m_ymin = std::numeric_limits<double>::max();
	m_zmin = std::numeric_limits<double>::max();
	m_xmax = std::numeric_limits<double>::min();
	m_ymax = std::numeric_limits<double>::min();
	m_zmax = std::numeric_limits<double>::min();
}

void SmtBoundingbox::append(glm::vec3 min, glm::vec3 max)
{
	if (min[0] < m_xmin)
		m_xmin = min[0];
	if (min[1] < m_ymin)
		m_ymin = min[1];
	if (min[2] < m_zmin)
		m_zmin = min[2];
	if (max[0] > m_xmax)
		m_xmax = max[0];
	if (max[1] > m_ymax)
		m_ymax = max[1];
	if (max[2] > m_zmax)
		m_zmax = max[2];
}