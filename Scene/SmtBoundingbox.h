#pragma once

#include <glm/vec3.hpp>

struct SmtBoundingbox
{
	SmtBoundingbox() { reset(); }
	void reset();
	void append(glm::vec3 min, glm::vec3 max);

	double m_xmin;
	double m_ymin;
	double m_zmin;
	double m_xmax;
	double m_ymax;
	double m_zmax;
};