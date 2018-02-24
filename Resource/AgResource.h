#pragma once

#include <stdint.h>

namespace ambergris {

	struct AgResource
	{
		AgResource() : m_handle(kInvalidHandle) {}

		static const uint16_t kInvalidHandle = UINT16_MAX;
		typedef uint16_t Handle;
		Handle m_handle;
	private:
		AgResource(const AgResource&);
		AgResource& operator=(const AgResource&);
	};
}