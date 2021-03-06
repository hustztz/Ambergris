#pragma once

#include "AgGeometry.h"
#include "AgVertexBuffer.h"
#include "AgIndexBuffer.h"
#include "AgMaterial.h"
#include "AgTexture.h"
#include "AgLight.h"
#include "AgCameraView.h"
#include "AgBoundingBox.h"
#include "AgCacheTransform.h"
#include "AgTextManager.h"
#include "Foundation/Singleton.h"

namespace ambergris {

	struct AgRenderResourceManager
	{
		bool init();
		void destroy();

		typedef AgResourcePool<AgVertexBuffer> VertexBufferPool;
		VertexBufferPool m_vertex_buffer_pool;
		typedef AgResourcePool<AgIndexBuffer> IndexBufferPool;
		IndexBufferPool m_index_buffer_pool;

		AgLightManager			m_lights;
		AgGeometryManager		m_geometries;
		AgMaterialManager		m_materials;
		AgShaderManager			m_shaders;
		AgTextureManager		m_textures;
		AgBoundingBoxManager	m_bboxManager;
		AgCameraViewManager		m_views;
		AgTransformManager		m_transforms;
		AgTextManager			m_text;
	private:
		AgRenderResourceManager();
		~AgRenderResourceManager();
		friend class Singleton<AgRenderResourceManager>;
		friend class BgfxRenderNode;
	};
}