#pragma once
#include "AgRenderSceneBridge.h"

namespace ambergris {

	class AgRenderPointCloudEvaluator : public AgRenderEvaluator
	{
	public:
		AgRenderPointCloudEvaluator() : AgRenderEvaluator() {}
		virtual ~AgRenderPointCloudEvaluator() {}

		virtual bool evaluate(const AgObject* pObject) override;
		virtual void bridgeRenderer(AgRenderer& renderer) const override{}
	};
}