#pragma once

#include <stdint.h>
#include <atomic>

namespace ambergris {

	struct AgResource
	{
		AgResource() : m_handle(kInvalidHandle), m_dirty(false) {}

		static const uint16_t kInvalidHandle = UINT16_MAX;
		typedef uint16_t Handle;
		Handle m_handle;
		std::atomic<bool>	m_dirty;
	private:
		AgResource(const AgResource&);
		AgResource& operator=(const AgResource&);
	};
}