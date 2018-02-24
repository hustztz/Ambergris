#pragma once
#include "AgRenderItem.h"
#include "Resource/AgMaterial.h"

#include <tinystl/allocator.h>
#include <tinystl/vector.h>
namespace stl = tinystl;

namespace ambergris {

	class AgRenderNode
	{
	public:
		AgRenderNode()
			: m_materialID(AgMaterial::MaterialType::E_LAMBERT)
		{
		}
		virtual ~AgRenderNode()
		{
		}

		virtual void DestroyGeometry();
		virtual bool Prepare() { return true; }
		virtual void Draw(bgfx::ViewId view) const;
		virtual bool AppendGeometry(
			const float* transform,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize);

		void SetMaterial(AgMaterial::MaterialType id) { m_materialID = id; }
		AgMaterial::MaterialType GetMaterial() const { return m_materialID; }
	protected:
		bgfx::VertexDecl	m_decl;
		AgMaterial::MaterialType	m_materialID;
		typedef stl::vector<AgRenderItem> RenderItemArray;
		RenderItemArray		m_items;
	};
}