#pragma once


#include "AgFxSystem.h"

namespace ambergris {

	class AgLightingSystem : public AgFxSystem
	{
	public:
		AgLightingSystem() {}
		virtual ~AgLightingSystem() {}

		virtual void begin() override;
		virtual AgShader::Handle getOverrideShader() const override { return AgShader::E_MESH_SHADING; }

	protected:
		void _SetPerFrameUniforms() const;
	};
}