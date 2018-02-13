#pragma once
#include "BgfxRenderItem.h"
#include "BgfxMaterialManager.h"

#include <tinystl/allocator.h>
#include <tinystl/vector.h>
namespace stl = tinystl;

namespace ambergris_bgfx {

	class BgfxRenderNode
	{
	public:
		BgfxRenderNode()
			: m_numTextures(0)
			, m_materialID(BgfxMaterialManager::MaterialIndex::E_LAMBERT)
		{
		}
		virtual ~BgfxRenderNode()
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

		void SetMaterial(BgfxMaterialManager::MaterialIndex id) { m_materialID = id; }
		BgfxMaterialManager::MaterialIndex GetMaterial() const { return m_materialID; }
	protected:
		struct Texture
		{
			uint32_t            m_flags;
			bgfx::UniformHandle m_sampler;
			bgfx::TextureHandle m_texture;
			uint8_t             m_stage;
		};

		Texture             m_textures[4];
		uint8_t             m_numTextures;
		bgfx::VertexDecl	m_decl;
		BgfxMaterialManager::MaterialIndex	m_materialID;
		typedef stl::vector<BgfxRenderItem> RenderItemArray;
		RenderItemArray		m_items;
	};
}