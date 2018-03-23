#pragma once

#include "AgRenderState.h"
#include "Resource/AgShader.h"

#include <stdint.h>

namespace ambergris {

	class AgRenderItem;
	class AgRenderNode;

	class AgFxSystem
	{
	public:
		AgFxSystem() {}
		virtual ~AgFxSystem() {}

		virtual void begin() {}
		virtual void end() {}
		virtual void setPerDrawUniforms(const AgShader* shader, const AgRenderItem* item) const {}
		virtual void updateTime(float time) {}
		virtual bool needTexture() const { return true; }
		virtual AgShader::Handle getOverrideShader() const = 0;
		virtual AgRenderState getOverrideStates() const { return AgRenderState(); }
	};
}