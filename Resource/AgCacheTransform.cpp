#include "AgCacheTransform.h"

#include <bgfx/bgfx.h>

using namespace ambergris;
using namespace ambergris::RealityComputing::Common;

//void AgCacheTransform::_ResetTransform()
//{
//	m_mtx[0] = m_mtx[5] = m_mtx[9] = m_mtx[15] = 1.0;
//	m_mtx[1] = m_mtx[2] = m_mtx[3] = m_mtx[4] =
//		m_mtx[6] = m_mtx[7] = m_mtx[8] = m_mtx[10] =
//		m_mtx[11] = m_mtx[12] = m_mtx[13] = m_mtx[14] = 0.0;
//}

void AgCacheTransform::scale(const RealityComputing::Common::RCVector3d& val)
{
	RCRotationMatrix rotationMtx = m_transform.getRotation();
	rotationMtx.scale(val);
	m_transform.setRotation(rotationMtx);

	/*if (!val)
		return;

	m_mtx[0] *= val[0];
	m_mtx[1] *= val[0];
	m_mtx[2] *= val[0];
	m_mtx[3] *= val[0];

	m_mtx[4] *= val[1];
	m_mtx[5] *= val[1];
	m_mtx[6] *= val[1];
	m_mtx[7] *= val[1];

	m_mtx[8] *= val[2];
	m_mtx[9] *= val[2];
	m_mtx[10] *= val[2];
	m_mtx[11] *= val[2];*/
}

void AgCacheTransform::setTransform(const double* mtx)
{
	if (mtx)
	{
		//memcpy_s(m_mtx, 16 * sizeof(double), mtx, 16 * sizeof(double));
		m_transform.fromColumnMajor(mtx);
	}
	else
	{
		//_ResetTransform();
		m_transform.identity();
	}
}

void AgCacheTransform::getFloatTransform(float* data) const
{
	if (!data)
		return;
	double dbTransform[16];
	m_transform.toColumnMajor(dbTransform);
	for (int i = 0; i < 16; i ++)
	{
		data[i] = (float)dbTransform[i];
	}
}

void AgTransformManager::setTransform(AgCacheTransform::Handle& id, const RCVector3d& translation, const RCVector3d& rotation, const RCVector3d& scale)
{
	AgCacheTransform* transform = get(id);
	if (transform)
	{
		transform->m_transform = RCTransform(RCRotationMatrix(rotation, scale), translation);
	}
	else
	{
		std::shared_ptr<AgCacheTransform> newTransform(new AgCacheTransform);
		newTransform->m_transform = RCTransform(RCRotationMatrix(rotation, scale), translation);
		id = append(newTransform);
	}
	m_dirty = true;
}
