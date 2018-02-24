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
		void Init(bx::FileReaderI* _reader);
		void Destroy();
		void Reset();
		void AppendNode(std::shared_ptr<AgRenderNode> renderNode);
		void Draw();
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