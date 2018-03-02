#pragma once

#include "AgMesh.h"
#include "Foundation/Singleton.h"
#include "Resource/AgResourcePool.h"

#include <tinystl/unordered_map.h>

namespace ambergris {

	struct AgSceneDatabase : public AgResourcePool<AgObject>
	{
	public:
		template<typename T>
		AgObject* allocate(bx::AllocatorI* allocator) {
			
			AgObject* pObject = AgResourcePool<AgObject>::allocate<T>(allocator);
			if (pObject)
			{
				uint32_t selectID = pObject->m_pick_id[0] + (pObject->m_pick_id[1] << 8) + (pObject->m_pick_id[2] << 16) + (255u << 24);
				if (m_select_id_map.find(selectID) == m_select_id_map.end())
				{
					m_select_id_map.insert(stl::make_pair<uint32_t, AgObject::Handle>(selectID, pObject->m_handle));
				}
				else
				{
					bx::RngMwc mwc;  // Random number generator
					pObject->m_pick_id[0] = mwc.gen() % 256;
					pObject->m_pick_id[1] = mwc.gen() % 256;
					pObject->m_pick_id[2] = mwc.gen() % 256;
					selectID = pObject->m_pick_id[0] + (pObject->m_pick_id[1] << 8) + (pObject->m_pick_id[2] << 16) + (255u << 24);
					m_select_id_map.insert(stl::make_pair<uint32_t, AgObject::Handle>(selectID, pObject->m_handle));
				}
			}
			return pObject;
		}
	private:
		AgSceneDatabase() {};
		~AgSceneDatabase() {}
		friend class Singleton<AgSceneDatabase>;
	public:
		typedef stl::unordered_map<uint32_t, AgObject::Handle> SelectMap;
		SelectMap		m_select_id_map;
		typedef stl::vector<AgObject::Handle> SelectResult;
		SelectResult	m_select_result;
	};
}