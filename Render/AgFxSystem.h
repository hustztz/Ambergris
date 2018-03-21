#pragma once

#include <stdint.h>
#include "Resource/AgShader.h"

namespace ambergris {

	struct AgShader;

	class AgFxSystem
	{
	public:
		AgFxSystem() {}
		virtual ~AgFxSystem() {}

		virtual void setPerFrameUniforms() const {}
		virtual void setPerDrawUniforms(const AgShader* shader, void* data) const {}
		virtual void auxiliaryDraw() {}
		virtual void updateTime(float time) {}
		virtual bool needTexture() const { return true; }
		virtual AgShader::Handle getOverrideShader() const = 0;
		virtual uint64_t getOverrideStates() const { return 0; }
	};
}