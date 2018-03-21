#pragma once

#include "AgFxSystem.h"

#define AMBERGRIS_STATE_OCCLUSION_QUERY (0            \
			| BGFX_STATE_DEPTH_TEST_LEQUAL       \
			| BGFX_STATE_CULL_CW     \
			)

namespace ambergris {

	class AgOcclusionSystem : public AgFxSystem
	{
	public:
		AgOcclusionSystem() {}
		virtual ~AgOcclusionSystem() {}

		virtual bool needTexture() const { return false; }
		virtual AgShader::Handle getOverrideShader() const override { return AgShader::E_SIMPLE_SHADER; }
		virtual uint64_t getOverrideStates() const override { return AMBERGRIS_STATE_OCCLUSION_QUERY; }
	};
}