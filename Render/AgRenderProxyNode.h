#pragma once
#include "AgRenderNode.h"

namespace ambergris {

	class AgRenderProxyNode : public AgRenderNode
	{
	public:
		AgRenderProxyNode()
			: AgRenderNode()
			, m_pItem(nullptr)
		{
		}
		virtual ~AgRenderProxyNode()
		{
			destroy();
		}

		virtual void draw(const ViewIdArray& views, const AgFxSystem* pFxSystem, int32_t occlusionCulling) const override;
		virtual bool appendGeometry(
			AgCacheTransform::Handle transform,
			AgMaterial::Handle material,
			AgBoundingbox::Handle bbox,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) override { return false; }
		virtual const AgRenderItem* getItem(uint16_t id) const override;

		void appendItem(const AgRenderItem* item) { m_pItem = item; }
		void setTransform(AgCacheTransform::Handle transform) { m_transform = transform; }
	protected:
		const AgRenderItem*		m_pItem;
		AgCacheTransform::Handle		m_transform;
	};
}