#pragma once

#include "Scene/AgMesh.h"
#include <memory>

namespace ambergris {
	class AgRenderer;
	class AgRenderNode;
	struct AgSceneDatabase;

	class AgRenderBaseEvaluator
	{
	public:
		struct EvaluateNode
		{
			EvaluateNode(AgMesh::Geometry geometry, std::shared_ptr<AgRenderNode> renderNode)
				: m_geometry(geometry), m_renderNode(renderNode) {}
			~EvaluateNode(){}

			AgMesh::Geometry				m_geometry;
			std::shared_ptr<AgRenderNode>	m_renderNode;
		};
		typedef  stl::vector<EvaluateNode> EvaluateNodeArr;

	public:
		AgRenderBaseEvaluator(EvaluateNodeArr& nodes) : m_evaluate_nodes(nodes){}
		virtual ~AgRenderBaseEvaluator() {}

		virtual bool Evaluate(const AgMesh& geomNode) = 0;
	protected:
		EvaluateNodeArr&			m_evaluate_nodes;
	};

	bool AgRenderSceneBridge(AgRenderer& renderer, const AgSceneDatabase& sceneDB);
}
