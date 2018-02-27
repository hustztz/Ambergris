#pragma once
#include "AgResource.h"

namespace ambergris {

	template<typename T, int S>
	class AgResourceContainer
	{
	public:
		virtual ~AgResourceContainer() { destroy(); }

		virtual void destroy() {}
		size_t getSize() const { return (size_t)S; }
		const T* get(AgResource::Handle id) const {
			if (id >= 0 && id < getSize())
				return &m_resource_arr[id];
			return nullptr;
		}
		T* get(AgResource::Handle id) {
			if (id >= 0 && id < getSize())
				return const_cast<T*>(&m_resource_arr[id]);
			return nullptr;
		}
	private:
		T		m_resource_arr[S];
	};
}