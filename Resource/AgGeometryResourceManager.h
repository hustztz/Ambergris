#pragma once

#include "AgVertexBuffer.h"
#include "AgIndexBuffer.h"
#include "AgResourceManager.h"
#include "Foundation/Singleton.h"

namespace ambergris {

	struct AgGeometryResourceManager
	{
		typedef AgResourceManager<AgVertexBuffer> VertexBufferPool;
		VertexBufferPool m_vertex_buffer_pool;
		typedef AgResourceManager<AgIndexBuffer> IndexBufferPool;
		IndexBufferPool m_index_buffer_pool;
	private:
		AgGeometryResourceManager() {}
		~AgGeometryResourceManager() {}
		friend class Singleton<AgGeometryResourceManager>;
		friend class BgfxRenderNode;
	};
}