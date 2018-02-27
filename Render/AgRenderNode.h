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
			: m_material_handle(AgMaterial::MaterialType::E_LAMBERT)
		{
			destroy();
		}
		virtual ~AgRenderNode()
		{
		}

		virtual void destroy();
		virtual bool prepare() { return true; }
		virtual void draw(bgfx::ViewId view) const;
		virtual bool appendGeometry(
			const float* transform,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize);

		void setMaterial(AgMaterial::Handle id) { m_material_handle = id; }
		AgMaterial::Handle getMaterial() const { return m_material_handle; }
		void setTexture(uint8_t slot, AgTexture::Handle tex_handle);
	protected:
		bgfx::VertexDecl	m_decl;
		AgMaterial::Handle	m_material_handle;
		AgTexture::Handle	m_texture[AgShader::MAX_TEXTURE_SLOT_COUNT];
		typedef stl::vector<AgRenderItem> RenderItemArray;
		RenderItemArray		m_items;
	};
}