#include "AgRenderer.h"
#include "AgRenderSceneBridge.h"
#include "Scene/AgSceneDatabase.h"

#include <thread>

namespace ambergris {

	AgRenderer::AgRenderer()
		: m_isEvaluating(false)
		, m_pipeline(m_viewPass)
	{
	}

	AgRenderer::~AgRenderer()
	{
	}

	void AgRenderer::evaluateScene()
	{
		if (Singleton<AgSceneDatabase>::instance().m_dirty && !Singleton<AgRenderer>::instance().m_isEvaluating)
		{
#if BGFX_CONFIG_MULTITHREADED
			std::thread bridgeThread(AgRenderSceneBridge);
			bridgeThread.detach();
#else
			AgRenderSceneBridge();
#endif // BGFX_CONFIG_MULTITHREADED
		}
	}
}