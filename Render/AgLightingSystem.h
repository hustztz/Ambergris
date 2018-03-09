#pragma once


#include "AgFxSystem.h"

namespace ambergris {

	class AgLightingSystem : public AgFxSystem
	{
	public:
		AgLightingSystem() {}
		virtual ~AgLightingSystem() {}

		virtual bool init() override { return true; }
		virtual void destroy() override {}
		virtual void setOverrideResource(const AgShader* shader, void* data) const override;
	};
}