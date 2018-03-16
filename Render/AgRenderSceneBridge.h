#pragma once
#include "Scene/AgObject.h"
#include "AgRenderNode.h"

#include <memory>
#include <vector>

namespace ambergris {

	struct AgRenderer;

	class AgRenderEvaluator
	{
	public:
		struct ObjectEvaluatorInfo
		{
			ObjectEvaluatorInfo(std::shared_ptr<AgRenderNode> renderNode, AgObject::Handle objectHandle)
				: m_pRenderNode(renderNode)
			{
				m_objectHandles.push_back(objectHandle);
			}
			virtual ~ObjectEvaluatorInfo() {}

			std::shared_ptr<AgRenderNode>	m_pRenderNode;
			std::vector<AgObject::Handle>	m_objectHandles;
		};
	public:
		AgRenderEvaluator() {}
		virtual ~AgRenderEvaluator() {}

		virtual bool evaluate(const AgObject* pObject) = 0;
		virtual void bridgeRenderer(AgRenderer& renderer) const = 0;
	};

	bool AgRenderSceneBridge();
}
