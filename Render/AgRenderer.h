#pragma once

#include "Foundation/Singleton.h"
#include "AgRenderPipeline.h"

namespace bx {
	struct FileReaderI;
}

namespace ambergris {

	class AgRenderer
	{
	public:
		void init(bx::FileReaderI* _reader);
		void destroy();
		void reset();
		void appendNode(std::shared_ptr<AgRenderNode> renderNode);
		void draw();
	protected:
	private:
		AgRenderer();
		~AgRenderer();
		friend class Singleton<AgRenderer>;
	private:
		AgRenderPipeline m_pipeline;
		bgfx::ViewId       m_viewId;
	};
}