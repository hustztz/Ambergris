#pragma once

#include "Foundation/Singleton.h"
#include "AgRenderPipeline.h"
#include "AgRenderQueue.h"

namespace ambergris {

	struct AgSceneDatabase;

	struct AgRenderer
	{
	public:
		void evaluateScene();
	private:
		AgRenderer();
		~AgRenderer();
		friend class Singleton<AgRenderer>;
	public:
		AgRenderPipeline m_pipeline;
		std::atomic<bool>		m_isEvaluating;
	};
}