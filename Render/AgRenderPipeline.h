#pragma once
#include "AgRenderNode.h"

#include <memory>

namespace ambergris {

	class AgRenderPipeline
	{
		struct AgRenderQueue
		{
			void clear();
			void appendNode(std::shared_ptr<AgRenderNode> renderNode);

			typedef stl::vector<std::shared_ptr<AgRenderNode>> RenderNodeArray;
			RenderNodeArray m_renderNodes;
		};
	public:
		enum Stage
		{
			E_STATIC_SCENE_OPAQUE = 0,
			E_STATIC_SCENE_TRANSPARENT,
			E_UI_OPAQUE,

			E_COUNT
		};
	protected:
		AgRenderPipeline();
		~AgRenderPipeline();
		friend class AgRenderer;

		void appendNode(Stage stage, std::shared_ptr<AgRenderNode> renderNode);
		void draw(bgfx::ViewId view) const;
		void clear(int stage);
		void reset();
	private:
	private:
		AgRenderQueue m_queues[E_COUNT];
	};

}