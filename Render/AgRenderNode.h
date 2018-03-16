#pragma once
#include "Resource/AgTexture.h"
#include "Resource/AgMaterial.h"

namespace ambergris {

	class AgFxSystem;
	class AgRenderItem;

	typedef std::vector<bgfx::ViewId> ViewIdArray;

	class AgRenderNode : public AgResource
	{
	public:
		AgRenderNode()
			: AgResource()
			, m_shader(AgShader::E_MESH_SHADING)
			, m_renderState(BGFX_STATE_DEFAULT)
		{
		}
		virtual ~AgRenderNode()
		{
			destroy();
		}

		virtual void destroy();
		virtual bool prepare() { return true; }
		virtual void draw(const ViewIdArray& views, AgFxSystem* pFxSystem, bool inOcclusionQuery) const = 0;
		virtual bool appendGeometry(
			const float* transform,
			AgMaterial::Handle material,
			const uint32_t* pick_id,
			const bgfx::VertexDecl& decl,
			const uint8_t* vertBuf, uint32_t vertSize,
			const uint16_t* indexBuf, uint32_t indexSize) = 0;
		virtual const AgRenderItem* getItem(uint16_t id) const = 0;
		virtual const float* getTransform(uint16_t id) const { return nullptr; }

		bool isTransparent() const { return ((m_renderState & BGFX_STATE_BLEND_ALPHA) != 0); } //TODO
		void setTexture(uint8_t slot, AgTexture::Handle tex_handle);
		void setShader(AgShader::Handle shader, uint64_t state) { m_shader = shader; m_renderState = state; }
	protected:
		void _SubmitTexture(const AgShader* shader) const;
		void _SubmitUniform(const AgShader* shader, const AgRenderItem*	item) const;
	protected:
		AgTexture::Handle	m_texture[AgShader::MAX_TEXTURE_SLOT_COUNT];
		bgfx::VertexDecl	m_decl;
		AgShader::Handle	m_shader;
		uint64_t			m_renderState;
	};
}