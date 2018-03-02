#pragma once

#include "Foundation/Singleton.h"
#include "AgRenderPipeline.h"
#include "AgRenderQueue.h"

namespace ambergris {

	struct AgRenderer
	{
	public:
		void draw();
	private:
		AgRenderer();
		~AgRenderer();
		friend class Singleton<AgRenderer>;
	public:
		AgRenderPipeline m_pipeline;
	};
}