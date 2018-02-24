#pragma once
#include "AgRenderNode.h"

#include <memory>

namespace ambergris {

	class AgRenderPipeline
	{
		struct AgRenderQueue
		{
			void Clear();
			void AppendNode(std::shared_ptr<AgRenderNode> renderNode);

			typedef stl::vector<std::shared_ptr<AgRenderNode>> RenderNodeArray;
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
		AgRenderPipeline();
		~AgRenderPipeline();
		friend class AgRenderer;

		void AppendNode(Stage stage, std::shared_ptr<AgRenderNode> renderNode);
		void Draw(bgfx::ViewId view) const;
		void Clear(int stage);
		void Reset();
	private:
	private:
		AgRenderQueue m_queues[E_COUNT];
	};

}