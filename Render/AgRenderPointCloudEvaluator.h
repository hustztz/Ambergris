#pragma once
#include "AgRenderSceneBridge.h"

namespace ambergris {

	template<typename T>
	class AgRenderPointCloudEvaluator : public AgRenderBaseEvaluator
	{
	public:
		AgRenderPointCloudEvaluator(EvaluateNodeArr& nodes) : AgRenderBaseEvaluator(nodes) {}
		virtual ~AgRenderPointCloudEvaluator() {}

		virtual bool Evaluate(const AgMesh& geomNode) override;
	};
}