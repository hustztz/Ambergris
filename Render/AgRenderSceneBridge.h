#pragma once

#include "Scene/AgMesh.h"
#include <memory>

namespace ambergris {
	class AgRenderNode;
	struct AgRenderer;
	struct AgSceneDatabase;

	class AgRenderBaseEvaluator
	{
	public:
		struct EvaluateNode
		{
			EvaluateNode(AgMesh::Geometry geometry, std::shared_ptr<AgRenderNode> renderNode)
				: m_geometry(geometry), m_pRenderNode(renderNode) {}
			~EvaluateNode(){}

			AgMesh::Geometry	m_geometry;
			std::shared_ptr<AgRenderNode>	m_pRenderNode;
		};
#ifdef USING_TINYSTL
		typedef  stl::vector<EvaluateNode> EvaluateNodeArr;
#else
		typedef  std::vector<EvaluateNode> EvaluateNodeArr;
#endif
		
	public:
		AgRenderBaseEvaluator(EvaluateNodeArr& nodes) : m_evaluate_nodes(nodes){}
		virtual ~AgRenderBaseEvaluator() {}

		virtual bool evaluate(const AgMesh& geomNode) = 0;
	protected:
		EvaluateNodeArr&			m_evaluate_nodes;
	};

	bool AgRenderSceneBridge();
}
