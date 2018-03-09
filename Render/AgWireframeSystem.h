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

		virtual bool init() override { return true; }
		virtual void destroy() override {}
		virtual void setOverrideResource(const AgShader* shader, void* data) const override {}
		virtual bool needTexture() const { return false; }
		virtual AgShader::Handle getOverrideShader() const override { return AgShader::E_SIMPLE_SHADER; }
		virtual uint64_t getOverrideStates() const override { return AMBERGRIS_STATE_WIREFRAME; }
	};
}