#pragma once

#include <bx/allocator.h>

namespace ambergris {

	struct TinyStlAllocator
	{
		static void* static_allocate(size_t _bytes);
		static void static_deallocate(void* _ptr, size_t /*_bytes*/);
	};

	extern bx::AllocatorI* getAllocator();
}