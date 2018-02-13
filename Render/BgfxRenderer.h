#pragma once

#include "Foundation/Singleton.h"
#include "BgfxRenderPipeline.h"

namespace bx {
	struct FileReaderI;
}

namespace ambergris_bgfx {

	class BgfxRenderer
	{
	public:
		void Init(bx::FileReaderI* _reader);
		void Destroy();
		void Reset();
		void AppendNode(BgfxRenderNode* renderNode);
		void Draw();
	protected:
	private:
		BgfxRenderer();
		~BgfxRenderer();
		friend class Singleton<BgfxRenderer>;
	private:
		BgfxRenderPipeline m_pipeline;
		bgfx::ViewId       m_viewId;
	};
}