#pragma once
#include "AgRenderItem.h"
#include "Resource/AgMaterial.h"

namespace ambergris {

	class AgFxSystem;

	class AgRenderNode : public AgResource
	{
	public:
		AgRenderNode()
			: AgResource()
			, m_material_handle(AgMaterial::MaterialType::E_LAMBERT)
		{
			destroy();
		}
		virtual ~AgRenderNode()
		{
			destroy();
		}

		virtual void destroy();
		virtual bool prepare() { return true; }
		virtual void draw(bgfx::ViewId view, AgFxSystem* pFxSystem, bool inOcclusionQuery);
		virtual bool appendGeometry(
			const float* transform,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize);

		void setMaterial(AgMaterial::Handle id) { m_material_handle = id; }
		AgMaterial::Handle getMaterial() const { return m_material_handle; }
		void setTexture(uint8_t slot, AgTexture::Handle tex_handle);
	protected:
		void _SubmitTexture(const AgShader* shader);
		void _SubmitUniform(const AgShader* shader, AgRenderItem*	item);
	protected:
		AgTexture::Handle	m_texture[AgShader::MAX_TEXTURE_SLOT_COUNT];
		bgfx::VertexDecl	m_decl;
		AgMaterial::Handle	m_material_handle;
		AgRenderItem		m_item;
	};
}