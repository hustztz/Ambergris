#pragma once
#include "AgRenderItem.h"
#include "AgRenderNode.h"

namespace ambergris {

	class AgRenderSingleNode : public AgRenderNode
	{
	public:
		AgRenderSingleNode()
			: AgRenderNode()
		{
		}
		virtual ~AgRenderSingleNode()
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
	protected:
		AgRenderItem		m_item;
	};
}