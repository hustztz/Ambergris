#include "AgRenderResourceManager.h"

namespace ambergris {

	AgRenderResourceManager::AgRenderResourceManager()
	{

	}
	AgRenderResourceManager::~AgRenderResourceManager()
	{
		destroy();
	}

	void AgRenderResourceManager::init(bx::FileReaderI* _reader)
	{
		if (!m_shaders.loadShader(_reader))
			return;
		m_materials.init();
	}

	void AgRenderResourceManager::destroy()
	{
		m_shaders.unloadShader();
		m_materials.destroy();
		m_textures.destroy();
	}
}