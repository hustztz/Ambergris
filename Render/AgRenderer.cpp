#include "AgRenderer.h"
#include "Resource/AgMaterial.h"

namespace ambergris {

	AgRenderer::AgRenderer()
	{
	}

	AgRenderer::~AgRenderer()
	{
	}

	void AgRenderer::Init(bx::FileReaderI* _reader)
	{
		Singleton<AgMaterialManager>::instance().Init(_reader);

		Reset();
		m_viewId = 0;
	}

	void AgRenderer::Destroy()
	{
		Singleton<AgMaterialManager>::instance().Destroy();
		m_pipeline.Reset();
	}

	void AgRenderer::Reset()
	{
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
		);
		m_pipeline.Reset();
	}
	
	void AgRenderer::Draw()
	{
		m_pipeline.Draw(m_viewId);
	}

	void AgRenderer::AppendNode(std::shared_ptr<AgRenderNode> renderNode)
	{
		if (!renderNode)
			return;
		switch (renderNode->GetMaterial())
		{
		case AgMaterial::E_LAMBERT:
		case AgMaterial::E_PHONG:
			m_pipeline.AppendNode(AgRenderPipeline::E_SCENE_OPAQUE, renderNode);
			break;
		case 3:
			m_pipeline.AppendNode(AgRenderPipeline::E_SCENE_TRANSPARENT, renderNode);
			break;
		default:
			m_pipeline.AppendNode(AgRenderPipeline::E_SCENE_OPAQUE, renderNode);
			break;
		}
	}
}