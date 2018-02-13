#include "BgfxRenderer.h"
#include "BgfxMaterialManager.h"

namespace ambergris_bgfx {

	BgfxRenderer::BgfxRenderer()
	{
	}

	BgfxRenderer::~BgfxRenderer()
	{
	}

	void BgfxRenderer::Init(bx::FileReaderI* _reader)
	{
		Singleton<ambergris_bgfx::BgfxMaterialManager>::instance().Init(_reader);

		Reset();
		m_viewId = 0;
	}

	void BgfxRenderer::Destroy()
	{
		Singleton<ambergris_bgfx::BgfxMaterialManager>::instance().Destroy();
		m_pipeline.Reset();
	}

	void BgfxRenderer::Reset()
	{
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
		);
		m_pipeline.Reset();
	}
	
	void BgfxRenderer::Draw()
	{
		m_pipeline.Draw(m_viewId);
	}

	void BgfxRenderer::AppendNode(BgfxRenderNode* renderNode)
	{
		if (!renderNode)
			return;
		switch (renderNode->GetMaterial())
		{
		case BgfxMaterialManager::E_LAMBERT:
		case BgfxMaterialManager::E_PHONG:
			m_pipeline.AppendNode(BgfxRenderPipeline::E_SCENE_OPAQUE, renderNode);
			break;
		case 3:
			m_pipeline.AppendNode(BgfxRenderPipeline::E_SCENE_TRANSPARENT, renderNode);
			break;
		default:
			m_pipeline.AppendNode(BgfxRenderPipeline::E_SCENE_OPAQUE, renderNode);
			break;
		}
	}
}