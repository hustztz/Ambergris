#pragma once
#include "AgResourcePool.h"

#include <common/RCTransform.h>

namespace ambergris {

	struct AgCacheTransform : public AgResource
	{
		AgCacheTransform() {}

		void setTransform(const double* mtx);
		void scale(const RealityComputing::Common::RCVector3d& val);
		void getFloatTransform(float* data) const;

		RealityComputing::Common::RCTransform             m_transform;

	protected:
		//void _ResetTransform();
	};

	class AgTransformManager : public AgResourcePool<AgCacheTransform>
	{
	public:
		AgTransformManager() : AgResourcePool<AgCacheTransform>(), m_dirty(false) {}

		void setTransform(AgCacheTransform::Handle& id, const RealityComputing::Common::RCVector3d& translation, const RealityComputing::Common::RCVector3d& rotation, const RealityComputing::Common::RCVector3d& scale);

		std::atomic<bool>		m_dirty;
	};
}