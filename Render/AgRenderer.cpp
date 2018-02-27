#include "AgRenderer.h"
#include "Resource/AgMaterial.h"

namespace ambergris {

	AgRenderer::AgRenderer()
	{
	}

	AgRenderer::~AgRenderer()
	{
	}

	void AgRenderer::init(bx::FileReaderI* _reader)
	{
		Singleton<AgMaterialManager>::instance().init(_reader);

		reset();
		m_viewId = 0;
	}

	void AgRenderer::destroy()
	{
		Singleton<AgMaterialManager>::instance().destroy();
		m_pipeline.reset();
	}

	void AgRenderer::reset()
	{
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
		);
		m_pipeline.reset();
	}
	
	void AgRenderer::draw()
	{
		m_pipeline.draw(m_viewId);
	}

	void AgRenderer::appendNode(std::shared_ptr<AgRenderNode> renderNode)
	{
		if (!renderNode)
			return;
		switch (renderNode->getMaterial())
		{
		case AgMaterial::E_LAMBERT:
		case AgMaterial::E_PHONG:
			m_pipeline.appendNode(AgRenderPipeline::E_STATIC_SCENE_OPAQUE, renderNode);
			break;
		case 3:
			m_pipeline.appendNode(AgRenderPipeline::E_STATIC_SCENE_TRANSPARENT, renderNode);
			break;
		default:
			m_pipeline.appendNode(AgRenderPipeline::E_STATIC_SCENE_OPAQUE, renderNode);
			break;
		}
	}
}