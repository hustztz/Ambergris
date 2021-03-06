#pragma once
#include "AgFxSystem.h"
#include "Resource/AgShader.h"

namespace ambergris {

	class AgHardwarePickingSystem : public AgFxSystem
	{
		static const uint8_t ID_DIM = 8;
	public:
		AgHardwarePickingSystem();
		~AgHardwarePickingSystem();

		bool init();
		void destroy();

		virtual void setPerDrawUniforms(const AgShader* shader, const AgRenderItem* item) const override;
		virtual bool needTexture() const override { return false; }
		virtual AgShader::Handle getOverrideShader() const override { return AgShader::E_PICKING_SHADER; }

		void updateView(bgfx::ViewId view_pass, float* invViewProj, float mouseXNDC, float mouseYNDC, float fov, float nearFrusm, float farFrusm);
		uint8_t acquireResult(bool isSinglePick);
		uint32_t readBlit(bgfx::ViewId view_pass);
		bgfx::TextureHandle getFBTexture() const { return m_pickingRT; }
		bool isPicked() const { return m_isPicked; }
		void endPick() { m_isPicked = false; }
	protected:
		bool _PrepareShader();
	private:
		bgfx::TextureHandle m_pickingRT;
		bgfx::TextureHandle m_pickingRTDepth;
		bgfx::TextureHandle m_blitTex;
		bgfx::FrameBufferHandle m_pickingFB;

		bool	m_homogeneousDepth;
		bool	m_isDX9;
		bool	m_isPicked;
		uint8_t m_blitData[ID_DIM*ID_DIM * 4]; // Read blit into this
	};
}