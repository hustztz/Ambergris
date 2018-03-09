#include "AgLightingSystem.h"
#include "Resource/AgShader.h"

namespace ambergris {

	/*virtual*/
	void AgLightingSystem::setOverrideResource(const AgShader* shader, void* data) const
	{
		if (!shader)
			return;

		float lightDir[4];
		lightDir[0] = -1.0;
		lightDir[1] = 1.0;
		lightDir[2] = 1.0;
		lightDir[3] = 1.0;
		bgfx::setUniform(shader->m_uniforms[0].uniform_handle, lightDir);
	}
}