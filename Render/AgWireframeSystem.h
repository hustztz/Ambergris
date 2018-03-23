#pragma once

#include "AgFxSystem.h"

#define AMBERGRIS_STATE_WIREFRAME (0            \
			| BGFX_STATE_PT_LINES       \
			| BGFX_STATE_WRITE_RGB     \
			| BGFX_STATE_LINEAA     \
			)

namespace ambergris {

	class AgWireframeSystem : public AgFxSystem
	{
	public:
		AgWireframeSystem() {}
		virtual ~AgWireframeSystem() {}

		virtual bool needTexture() const { return false; }
		virtual AgShader::Handle getOverrideShader() const override { return AgShader::E_SIMPLE_SHADER; }
		virtual AgRenderState getOverrideStates() const override {
			AgRenderState renderState;
			renderState.m_state = AMBERGRIS_STATE_WIREFRAME;
			return renderState;
		}
	};
}