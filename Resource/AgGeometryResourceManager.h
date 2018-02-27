#pragma once

#include "AgVertexBuffer.h"
#include "AgIndexBuffer.h"
#include "AgResourcePool.h"
#include "Foundation/Singleton.h"

namespace ambergris {

	struct AgGeometryResourceManager
	{
		typedef AgResourcePool<AgVertexBuffer> VertexBufferPool;
		VertexBufferPool m_vertex_buffer_pool;
		typedef AgResourcePool<AgIndexBuffer> IndexBufferPool;
		IndexBufferPool m_index_buffer_pool;
	private:
		AgGeometryResourceManager() {}
		~AgGeometryResourceManager() {}
		friend class Singleton<AgGeometryResourceManager>;
		friend class BgfxRenderNode;
	};
}