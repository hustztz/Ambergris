#include "AgObject.h"

#include "Resource/AgRenderResourceManager.h"

using namespace ambergris;

/*virtual*/
void AgObject::setGlobalPosition(double* val)
{
	AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
	if (!transform)
		return;

	transform->m_mtx[12] = val[0];
	transform->m_mtx[13] = val[1];
	transform->m_mtx[14] = val[2];
}