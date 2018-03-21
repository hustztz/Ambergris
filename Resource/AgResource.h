#pragma once

#include <stdint.h>
#include <atomic>

namespace ambergris {

	struct AgResource
	{
		AgResource() : m_handle(kInvalidHandle), m_prepared(false) {}

		static const uint16_t kInvalidHandle = UINT16_MAX;
		typedef uint16_t Handle;
		Handle m_handle;
		std::atomic<bool>	m_prepared;
	private:
		AgResource(const AgResource&);
		AgResource& operator=(const AgResource&);
	};
}