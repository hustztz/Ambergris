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
		AgRenderEvaluator() {}
		virtual ~AgRenderEvaluator() {}

		virtual bool evaluate(AgRenderer& renderer, AgDrawInfo drawInfo) = 0;
	};

	bool AgRenderSceneBridge();
}
