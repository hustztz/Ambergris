#include "AgCacheTransform.h"

#include <bgfx/bgfx.h>

using namespace ambergris;


void AgCacheTransform::_ResetTransform()
{
	m_mtx[0] = m_mtx[5] = m_mtx[9] = m_mtx[15] = 1.0;
	m_mtx[1] = m_mtx[2] = m_mtx[3] = m_mtx[4] =
		m_mtx[6] = m_mtx[7] = m_mtx[8] = m_mtx[10] =
		m_mtx[11] = m_mtx[12] = m_mtx[13] = m_mtx[14] = 0.0;
}


void AgCacheTransform::setTransform(double* mtx)
{
	if (mtx)
	{
		memcpy_s(m_mtx, 16 * sizeof(double), mtx, 16 * sizeof(double));
	}
	else
	{
		_ResetTransform();
	}
}

void AgCacheTransform::getFloatTransform(float* data) const
{
	if (!data)
		return;
	for (int i = 0; i < 16; i ++)
	{
		data[i] = (float)m_mtx[i];
	}
}
