#include "BgfxBatchRenderEvaluator.h"
#include "BgfxRenderer.h"
#include "Scene\SceneGeometryResource.h"
#include "Scene/BgfxSceneDatabase.h"

using namespace ambergris;

namespace ambergris_bgfx {

	/*virtual*/
	template<typename T>
	bool BgfxBatchRenderEvaluator<T>::Evaluate(const ambergris::BgfxNodeHeirarchy& geomNode)
	{
		
		return true;
	}
}