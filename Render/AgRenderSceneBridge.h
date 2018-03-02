#pragma once

#include "Scene/AgMesh.h"

namespace ambergris {
	class AgRenderNode;
	struct AgRenderer;
	struct AgSceneDatabase;

	class AgRenderBaseEvaluator
	{
	public:
		struct EvaluateNode
		{
			EvaluateNode(AgMesh::Geometry geometry, AgRenderNode* renderNode)
				: m_geometry(geometry), m_pRenderNode(renderNode) {}
			~EvaluateNode(){}

			AgMesh::Geometry	m_geometry;
			AgRenderNode*		m_pRenderNode;
		};
		typedef  stl::vector<EvaluateNode> EvaluateNodeArr;

	public:
		AgRenderBaseEvaluator(EvaluateNodeArr& nodes) : m_evaluate_nodes(nodes){}
		virtual ~AgRenderBaseEvaluator() {}

		virtual bool evaluate(const AgMesh& geomNode) = 0;
	protected:
		EvaluateNodeArr&			m_evaluate_nodes;
	};

	bool AgRenderSceneBridge(AgRenderer& renderer, const AgSceneDatabase& sceneDB);
}
