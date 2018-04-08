#pragma once
#include "AgRenderSceneBridge.h"

namespace ambergris {

	class AgRenderPointCloudEvaluator : public AgRenderEvaluator
	{
	public:
		AgRenderPointCloudEvaluator() : AgRenderEvaluator() {}
		virtual ~AgRenderPointCloudEvaluator() {}

		virtual bool evaluate(AgRenderer& renderer, AgDrawInfo drawInfo) override;
	};
}