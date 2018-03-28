#pragma once

#include <stdint.h>
#include <atomic>

namespace ambergris {

	struct AgResource
	{
		AgResource() : m_handle(kInvalidHandle) {}

		static const uint16_t kInvalidHandle = UINT16_MAX;
		//typedef std::atomic<uint16_t> Handle;
		typedef uint16_t Handle;
		Handle m_handle;
	private:
		AgResource(const AgResource&);
		AgResource& operator=(const AgResource&);
	};
}