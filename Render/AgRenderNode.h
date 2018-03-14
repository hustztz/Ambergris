#pragma once
#include "Resource/AgMaterial.h"
#include "Resource/AgTexture.h"

namespace ambergris {

	class AgFxSystem;
	class AgRenderItem;

	typedef std::vector<bgfx::ViewId> ViewIdArray;

	class AgRenderNode : public AgResource
	{
	public:
		AgRenderNode()
			: AgResource()
			, m_material_handle(AgMaterial::MaterialType::E_LAMBERT)
		{
		}
		virtual ~AgRenderNode()
		{
			destroy();
		}

		virtual void destroy();
		virtual bool prepare() { return true; }
		virtual void draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery) = 0;
		virtual bool appendGeometry(
			const float* transform,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) = 0;
		virtual const AgRenderItem* findItem(const uint32_t* pick_id) const = 0;

		void setMaterial(AgMaterial::Handle id) { m_material_handle = id; }
		AgMaterial::Handle getMaterial() const { return m_material_handle; }
		void setTexture(uint8_t slot, AgTexture::Handle tex_handle);
	protected:
		void _SubmitTexture(const AgShader* shader);
		void _SubmitUniform(const AgShader* shader, const AgRenderItem*	item);
	protected:
		AgTexture::Handle	m_texture[AgShader::MAX_TEXTURE_SLOT_COUNT];
		bgfx::VertexDecl	m_decl;
		AgMaterial::Handle	m_material_handle;
	};
}