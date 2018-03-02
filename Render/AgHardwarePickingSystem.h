#pragma once

#include "Resource/AgShader.h"

namespace ambergris {

	class AgHardwarePickingSystem
	{
		static const uint8_t ID_DIM = 8;
	public:
		AgHardwarePickingSystem();
		~AgHardwarePickingSystem();

		void init();
		void update(bgfx::ViewId view_pass, float* invViewProj, float mouseXNDC, float mouseYNDC);
		uint8_t acquireResult(bool isSinglePick = true);
		uint32_t readBlit(bgfx::ViewId view_pass);

		bool isPicked() const { return m_isPicked; }
		void Picked() { m_isPicked = true; }

		static AgShader::Handle getPickingShader() { return AgShader::E_PICKING_SHADER;	}
		static uint64_t getPickingStates() { return BGFX_STATE_DEFAULT;	}
	protected:
	private:
		bgfx::TextureHandle m_pickingRT;
		bgfx::TextureHandle m_pickingRTDepth;
		bgfx::TextureHandle m_blitTex;
		bgfx::FrameBufferHandle m_pickingFB;

		float	m_fov;
		bool	m_homogeneousDepth;
		bool	m_isDX9;
		bool	m_isPicked;
		uint8_t m_blitData[ID_DIM*ID_DIM * 4]; // Read blit into this
	};
}