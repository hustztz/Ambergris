#pragma once
#include "BgfxRenderNode.h"
#include <boost/ptr_container/ptr_vector.hpp>

namespace ambergris_bgfx {

	class BgfxRenderPipeline
	{
		struct BgfxRenderQueue
		{
			void Clear();
			void AppendNode(BgfxRenderNode* renderNode);

			typedef boost::ptr_vector<BgfxRenderNode> RenderNodeArray;
			RenderNodeArray m_renderNodes;
		};
	public:
		enum Stage
		{
			E_SCENE_OPAQUE = 0,
			E_SCENE_TRANSPARENT,
			E_UI_OPAQUE,

			E_COUNT
		};
	protected:
		BgfxRenderPipeline();
		~BgfxRenderPipeline();
		friend class BgfxRenderer;

		void AppendNode(Stage stage, BgfxRenderNode* renderNode);
		void Draw(bgfx::ViewId view) const;
		void Clear(int stage);
		void Reset();
	private:
	private:
		BgfxRenderQueue m_queues[E_COUNT];
	};

}