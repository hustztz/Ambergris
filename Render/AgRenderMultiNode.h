#pragma once
#include "AgRenderNode.h"
#include "AgRenderItem.h"

namespace ambergris {

	class AgRenderMultiNode : public AgRenderNode
	{
	public:
		AgRenderMultiNode()
			: AgRenderNode()
		{
		}
		virtual ~AgRenderMultiNode()
		{
			destroy();
		}

		virtual void destroy() override;
		virtual void draw(const ViewIdArray& views, const AgFxSystem* pFxSystem, int32_t occlusionCulling) const override;
		virtual bool appendGeometry(
			AgCacheTransform::Handle transform,
			AgMaterial::Handle material,
			AgBoundingbox::Handle bbox,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) override;
		virtual const AgRenderItem* getItem(uint16_t id) const override;
		virtual AgCacheTransform::Handle getTransform(uint16_t id) const override;
	protected:
#ifdef USING_TINYSTL
		typedef stl::vector<AgRenderItem*> RenderItemArray;
#else
		typedef std::vector<AgRenderItem*> RenderItemArray;
#endif
		RenderItemArray		m_items;
	};
}