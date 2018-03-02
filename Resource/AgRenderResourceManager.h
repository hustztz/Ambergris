#pragma once

#include "AgVertexBuffer.h"
#include "AgIndexBuffer.h"
#include "AgMaterial.h"
#include "AgTexture.h"
#include "Foundation/Singleton.h"

namespace bx {
	struct FileReaderI;
}

namespace ambergris {

	struct AgRenderResourceManager
	{
		void init(bx::FileReaderI* _reader);
		void destroy();

		typedef AgResourcePool<AgVertexBuffer> VertexBufferPool;
		VertexBufferPool m_vertex_buffer_pool;
		typedef AgResourcePool<AgIndexBuffer> IndexBufferPool;
		IndexBufferPool m_index_buffer_pool;

		AgMaterialManager		m_materials;
		AgShaderManager			m_shaders;
		AgTextureManager		m_textures;
	private:
		AgRenderResourceManager();
		~AgRenderResourceManager();
		friend class Singleton<AgRenderResourceManager>;
		friend class BgfxRenderNode;
	};
}