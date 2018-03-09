#pragma once
#include "AgResource.h"
//#include <mutex>
#include <bx/allocator.h>

#ifdef USING_TINYSTL
#include <tinystl/allocator.h>
#include <tinystl/vector.h>
namespace stl = tinystl;
#else
#include <vector>
#endif

namespace ambergris {

	template<typename T>
	class AgResourcePool
	{
	public:
		virtual ~AgResourcePool() { destroy(); }

		virtual void destroy() {
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
				if(res == get(res->m_handle))
					return res;
			}
			
			res->m_handle = (AgResource::Handle)getSize();
			//std::lock_guard<std::mutex> lck(m_mutex);
			m_resource_arr.push_back(res);
			return res;
		}
		/*template<>
		T* allocate<T>(bx::AllocatorI* allocator) {
			return allocate<T>(allocator);
		}*/

		size_t getSize() const { return m_resource_arr.size(); }
		const T* get(AgResource::Handle id) const {
			//std::lock_guard<std::mutex> lck(m_mutex);
			if (id >= 0 && id < getSize())
				return m_resource_arr[id];
			return nullptr;
		}
		T* get(AgResource::Handle id) {
			//std::lock_guard<std::mutex> lck(m_mutex);
			if (id >= 0 && id < getSize())
				return m_resource_arr[id];
			return nullptr;
		}
	protected:
#ifdef USING_TINYSTL
		typedef stl::vector<T*> Resources;
#else
		typedef std::vector<T*> Resources;
#endif
		Resources		m_resource_arr;
		//mutable std::mutex		m_mutex;
	};
}