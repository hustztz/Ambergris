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

		virtual void draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery) override;
		virtual bool appendGeometry(
			const float* transform,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize);
		virtual const AgRenderItem* findItem(const uint32_t* pick_id) const override;

		void appendItem(const AgRenderItem* item) { m_pItem = item; }
	protected:
		const AgRenderItem*		m_pItem;
	};
}