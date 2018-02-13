#pragma once
#include "BgfxRenderEvaluator.h"

namespace ambergris_bgfx {

	template<typename T>
	class BgfxBatchRenderEvaluator : public BgfxRenderEvaluator<T>
	{
	public:
		BgfxBatchRenderEvaluator(EvaluateNodeArr& nodes) : BgfxRenderEvaluator<T>(nodes) {}
		virtual ~BgfxBatchRenderEvaluator() {}

		virtual bool Evaluate(const ambergris::BgfxNodeHeirarchy& geomNode) override;
	};
}