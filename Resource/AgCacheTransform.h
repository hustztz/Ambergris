#pragma once
#include "AgResourcePool.h"

namespace ambergris {

	struct AgCacheTransform : public AgResource
	{
		AgCacheTransform() {}

		void setTransform(double* mtx);
		void getFloatTransform(float* data) const;

		double		m_mtx[16];

	protected:
		void _ResetTransform();
	};

	class AgTransformManager : public AgResourcePool<AgCacheTransform>
	{
	public:
		AgTransformManager() : AgResourcePool<AgCacheTransform>(), m_dirty(false) {}

		std::atomic<bool>		m_dirty;
	};
}