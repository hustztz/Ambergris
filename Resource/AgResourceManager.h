#pragma once
#include "AgResource.h"

#include <bx/allocator.h>

#include <tinystl/allocator.h>
#include <tinystl/vector.h>
namespace stl = tinystl;

namespace ambergris {

	template<typename T>
	class AgResourceManager
	{
	public:
		virtual ~AgResourceManager() { Destroy(); }

		virtual void Destroy() {
			for (Resources::iterator itm = m_resource_arr.begin(); itm != m_resource_arr.end(); ++ itm)
			{
				T* pItm = *itm;
				if (pItm)
					delete pItm;
			}
			m_resource_arr.clear();
		}
		template<typename CT>
		T* allocate(bx::AllocatorI* allocator) {
			T* res = (T*)BX_NEW(allocator, CT);
			if (!res)
				return nullptr;

			if (AgResource::kInvalidHandle != res->m_handle)
			{
				if(res == Get(res->m_handle))
					return res;
			}
			
			res->m_handle = (AgResource::Handle)GetSize();
			m_resource_arr.push_back(res);
			return res;
		}
		/*template<>
		T* allocate<T>(bx::AllocatorI* allocator) {
			return allocate<T>(allocator);
		}*/

		size_t GetSize() const { return m_resource_arr.size(); }
		const T* Get(AgResource::Handle id) const {
			if (id >= 0 && id < GetSize())
				return m_resource_arr[id];
			return nullptr;
		}
		T* Get(AgResource::Handle id) {
			if (id >= 0 && id < GetSize())
				return const_cast<T*>(m_resource_arr[id]);
			return nullptr;
		}
	private:
		typedef stl::vector<T*> Resources;
		Resources		m_resource_arr;
	};
}