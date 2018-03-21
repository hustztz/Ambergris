#include "AgRenderResourceManager.h"

namespace ambergris {

	AgRenderResourceManager::AgRenderResourceManager()
	{

	}
	AgRenderResourceManager::~AgRenderResourceManager()
	{
		destroy();
	}

	bool AgRenderResourceManager::init()
	{
		if (!m_shaders.loadBasicShader())
			return false;
		m_materials.init();
		if (!m_text.init())
			return false;
		return true;
	}

	void AgRenderResourceManager::destroy()
	{
		m_shaders.unloadShader();
		m_materials.destroy();
		m_textures.destroy();
		m_text.destroy();
	}
}