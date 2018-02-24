#pragma once
#include "AgResource.h"

namespace ambergris {

	template<typename T, int S>
	class AgResourceContainer
	{
	public:
		virtual ~AgResourceContainer() { Destroy(); }

		virtual void Destroy() {}
		size_t GetSize() const { return (size_t)S; }
		const T* Get(AgResource::Handle id) const {
			if (id >= 0 && id < GetSize())
				return &m_resource_arr[id];
			return nullptr;
		}
		T* Get(AgResource::Handle id) {
			if (id >= 0 && id < GetSize())
				return const_cast<T*>(&m_resource_arr[id]);
			return nullptr;
		}
	private:
		T		m_resource_arr[S];
	};
}