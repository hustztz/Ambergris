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
		virtual void draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery) override;
		virtual bool appendGeometry(
			const float* transform,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) override;
		virtual const AgRenderItem* findItem(const uint32_t* pick_id) const override;
	protected:
		AgRenderItem		m_item;
	};
}