#pragma once
#include "AgRenderNode.h"

namespace ambergris {

	class AgRenderCompoundNode : public AgRenderNode
	{
	public:
		AgRenderCompoundNode()
			: AgRenderNode()
		{
		}
		virtual ~AgRenderCompoundNode()
		{
			destroy();
		}

		virtual void destroy() override;
		virtual void draw(bgfx::ViewId view, AgFxSystem* pFxSystem, bool inOcclusionQuery) override;
		virtual bool appendGeometry(
			const float* transform,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) override;
	protected:
#ifdef USING_TINYSTL
		typedef stl::vector<AgRenderItem> RenderItemArray;
#else
		typedef std::vector<AgRenderItem> RenderItemArray;
#endif
		RenderItemArray		m_items;
	};
}