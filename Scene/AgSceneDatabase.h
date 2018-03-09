#pragma once

#include "AgMesh.h"
#include "Foundation/Singleton.h"
#include "Resource/AgResourcePool.h"

#ifdef USING_TINYSTL
#include <tinystl/unordered_map.h>
#else
#include <unordered_map>
#endif


namespace ambergris {

	struct AgSceneDatabase : public AgResourcePool<AgObject>
	{
	public:
		template<typename T>
		AgObject* allocate(bx::AllocatorI* allocator) {
			
			AgObject* pObject = AgResourcePool<AgObject>::allocate<T>(allocator);
			AgMesh* pMesh = dynamic_cast<AgMesh*>(pObject);
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
					bx::RngMwc mwc;  // Random number generator
					pMesh->m_pick_id[0] = mwc.gen() % 256;
					pMesh->m_pick_id[1] = mwc.gen() % 256;
					pMesh->m_pick_id[2] = mwc.gen() % 256;
					selectID = pMesh->m_pick_id[0] + (pMesh->m_pick_id[1] << 8) + (pMesh->m_pick_id[2] << 16) + (255u << 24);
#ifdef USING_TINYSTL
					m_select_id_map.insert(stl::make_pair<uint32_t, AgObject::Handle>(selectID, pMesh->m_handle));
#else
					m_select_id_map[selectID] = pMesh->m_handle;
#endif		
				}
			}
			return pObject;
		}
	private:
		AgSceneDatabase() : AgResourcePool<AgObject>(), m_dirty(false) {};
		~AgSceneDatabase() {}
		friend class Singleton<AgSceneDatabase>;
	public:
#ifdef USING_TINYSTL
		typedef stl::unordered_map<uint32_t, AgMesh::Handle> SelectMap;
#else
		typedef std::unordered_map<uint32_t, AgMesh::Handle> SelectMap;
#endif
		SelectMap		m_select_id_map;
#ifdef USING_TINYSTL
		typedef stl::vector<AgMesh::Handle> SelectResult;
#else
		typedef std::vector<AgMesh::Handle> SelectResult;
#endif
		SelectResult	m_select_result;

		std::atomic<bool>		m_dirty;
	};
}