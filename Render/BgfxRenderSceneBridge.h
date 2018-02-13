#pragma once

#include <vector>

namespace ambergris {
	struct BgfxSceneDatabase;
	struct BgfxNodeHeirarchy;
}

namespace ambergris_bgfx {
	class BgfxRenderer;
	class BgfxRenderNode;

	class BgfxRenderBaseEvaluator
	{
	public:
		struct EvaluateNode
		{
			EvaluateNode(int mesh_handle, BgfxRenderNode* renderNode) : m_mesh_handle(mesh_handle), m_renderNode(renderNode) {}

			int				m_mesh_handle;
			BgfxRenderNode* m_renderNode;
		};
		typedef  std::vector<EvaluateNode> EvaluateNodeArr;

	public:
		BgfxRenderBaseEvaluator(EvaluateNodeArr& nodes) : m_evaluate_nodes(nodes){}
		virtual ~BgfxRenderBaseEvaluator() {}

		virtual bool Evaluate(const ambergris::BgfxNodeHeirarchy& geomNode) = 0;
	protected:
		EvaluateNodeArr&			m_evaluate_nodes;
	};

	bool BgfxRenderSceneBridge(BgfxRenderer& renderer, const ambergris::BgfxSceneDatabase& sceneDB);
}
