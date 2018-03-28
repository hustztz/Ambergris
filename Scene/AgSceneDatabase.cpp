#include "AgSceneDatabase.h"
#include "Resource/AgRenderResourceManager.h"

#include "BGFX/entry/entry.h"//TODO

namespace ambergris {

	/*virtual*/
	AgObject::Handle AgSceneDatabase::appendObject(std::shared_ptr<AgObject> object)
	{
		AgObject::Handle handle = m_objectManager.append(object);
		if (AgObject::kInvalidHandle == handle)
			return handle;

		std::shared_ptr<AgMesh> pMesh = std::dynamic_pointer_cast<AgMesh>(object);
		if (pMesh)
		{
			uint32_t selectID = pMesh->m_pick_id[0] + (pMesh->m_pick_id[1] << 8) + (pMesh->m_pick_id[2] << 16) + (255u << 24);
			if (m_select_id_map.find(selectID) == m_select_id_map.end())
			{
#ifdef USING_TINYSTL
				m_select_id_map.insert(stl::make_pair<uint32_t, AgObject::Handle>(selectID, pMesh->m_handle));
#else
				m_select_id_map[selectID] = pMesh->m_handle;
#endif
			}
			else
			{
				// Re-hash
				pMesh->m_pick_id[0] = (pMesh->m_pick_id[0] + 1) % 256;
				pMesh->m_pick_id[1] = (pMesh->m_pick_id[1] + 1) % 256;
				pMesh->m_pick_id[2] = (pMesh->m_pick_id[2] + 1) % 256;
				selectID = pMesh->m_pick_id[0] + (pMesh->m_pick_id[1] << 8) + (pMesh->m_pick_id[2] << 16) + (255u << 24);
#ifdef USING_TINYSTL
				m_select_id_map.insert(stl::make_pair<uint32_t, AgObject::Handle>(selectID, pMesh->m_handle));
#else
				m_select_id_map[selectID] = pMesh->m_handle;
#endif		
			}
		}

		return handle;
	}
}