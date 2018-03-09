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

		virtual bool init() = 0;
		virtual void destroy() = 0;
		virtual void setOverrideResource(const AgShader* shader, void* data) const = 0;
		virtual void auxiliaryDraw() {}
		virtual void updateTime(float time) {}
		virtual bool needTexture() const { return true; }
		virtual AgShader::Handle getOverrideShader() const { return AgShader::E_COUNT; }
		virtual uint64_t getOverrideStates() const { return BGFX_STATE_DEFAULT; }
	};
}