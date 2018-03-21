#include "AgLightingSystem.h"
#include "Resource/AgShader.h"
#include "Resource/AgRenderResourceManager.h"

namespace ambergris {

	/*virtual*/
	void AgLightingSystem::setPerFrameUniforms() const
	{
		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(getOverrideShader());
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