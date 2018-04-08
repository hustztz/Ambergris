#include "AgObject.h"

#include "Resource/AgRenderResourceManager.h"
#include "BGFX/entry/entry.h"//TODO

using namespace ambergris;

/*virtual*/
//void AgObject::setGlobalPosition(double* val)
//{
//	AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
//	if (!transform)
//		return;
//
//	transform->m_mtx[12] = val[0];
//	transform->m_mtx[13] = val[1];
//	transform->m_mtx[14] = val[2];
//}

RealityComputing::Common::RCTransform AgObject::getTransform() const
{
	const AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
	if (!transform || AgCacheTransform::kInvalidHandle == transform->m_handle)
		return RealityComputing::Common::RCTransform();

	return transform->m_transform;
}

AgCacheTransform::Handle AgObject::setTransform(double* val)
{
	const AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
	if (!transform || AgCacheTransform::kInvalidHandle == transform->m_handle)
	{
		std::shared_ptr<AgCacheTransform> cacheTransform(BX_NEW(entry::getAllocator(), AgCacheTransform));
		if(val)
			cacheTransform->setTransform(val);
		m_global_transform_h = Singleton<AgRenderResourceManager>::instance().m_transforms.append(cacheTransform);
	}

	return transform->m_handle;
}