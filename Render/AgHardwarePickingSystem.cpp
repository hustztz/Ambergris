#include "AgHardwarePickingSystem.h"
#include "AgRenderItem.h"
#include "Resource/AgRenderResourceManager.h"
#include "Resource/AgShaderUtils.h"
#include "Scene/AgSceneDatabase.h"

#include "bx/bx.h"
#include "bx/math.h"
#include <map>

#define PICKING_RESULT_THRESHOULD 5

namespace ambergris {

	AgHardwarePickingSystem::AgHardwarePickingSystem()
		: m_pickingRT(BGFX_INVALID_HANDLE)
		, m_pickingRTDepth(BGFX_INVALID_HANDLE)
		, m_blitTex(BGFX_INVALID_HANDLE)
		, m_pickingFB(BGFX_INVALID_HANDLE)
		, m_isPicked(false)
	{
		const bgfx::Caps* caps = bgfx::getCaps();
		m_homogeneousDepth = caps->homogeneousDepth;
		m_isDX9 = (bgfx::RendererType::Direct3D9 == caps->rendererType);
		init();
	}

	AgHardwarePickingSystem::~AgHardwarePickingSystem()
	{
		destroy();
	}

	bool AgHardwarePickingSystem::_PrepareShader()
	{
		{
			AgShader* picking_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_PICKING_SHADER);
			if (!picking_shader)
				return false;

			if (AgShader::kInvalidHandle == picking_shader->m_handle)
			{
				picking_shader->m_program = shaderUtils::loadProgram("vs_picking", "fs_picking_id");
				if (!bgfx::isValid(picking_shader->m_program))
					return false;

				picking_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_id", bgfx::UniformType::Vec4); // ID for drawing into ID buffer
				picking_shader->m_handle = AgShader::E_PICKING_SHADER;
			}
		}
		
		{
			AgShader* picking_instance_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_PICKING_INSTANCE_SHADER);
			if (!picking_instance_shader)
				return false;

			if (AgShader::kInvalidHandle == picking_instance_shader->m_handle)
			{
				picking_instance_shader->m_program = shaderUtils::loadProgram("vs_pick_instancing", "fs_picking_id");
				if (!bgfx::isValid(picking_instance_shader->m_program))
					return false;

				picking_instance_shader->m_handle = AgShader::E_PICKING_INSTANCE_SHADER;
			}
		}
		return true;
	}

	bool AgHardwarePickingSystem::init()
	{
		if (!_PrepareShader())
		{
			printf("Failed to load picking shaders.\n");
			return false;
		}

		// Set up ID buffer, which has a color target and depth buffer
		m_pickingRT = bgfx::createTexture2D(ID_DIM, ID_DIM, false, 1, bgfx::TextureFormat::RGBA8, 0
			| BGFX_TEXTURE_RT
			| BGFX_TEXTURE_MIN_POINT
			| BGFX_TEXTURE_MAG_POINT
			| BGFX_TEXTURE_MIP_POINT
			| BGFX_TEXTURE_U_CLAMP
			| BGFX_TEXTURE_V_CLAMP
		);
		m_pickingRTDepth = bgfx::createTexture2D(ID_DIM, ID_DIM, false, 1, bgfx::TextureFormat::D24S8, 0
			| BGFX_TEXTURE_RT
			| BGFX_TEXTURE_MIN_POINT
			| BGFX_TEXTURE_MAG_POINT
			| BGFX_TEXTURE_MIP_POINT
			| BGFX_TEXTURE_U_CLAMP
			| BGFX_TEXTURE_V_CLAMP
		);

		// CPU texture for blitting to and reading ID buffer so we can see what was clicked on.
		// Impossible to read directly from a render target, you *must* blit to a CPU texture
		// first. Algorithm Overview: Render on GPU -> Blit to CPU texture -> Read from CPU
		// texture.
		m_blitTex = bgfx::createTexture2D(ID_DIM, ID_DIM, false, 1, bgfx::TextureFormat::RGBA8, 0
			| BGFX_TEXTURE_BLIT_DST
			| BGFX_TEXTURE_READ_BACK
			| BGFX_TEXTURE_MIN_POINT
			| BGFX_TEXTURE_MAG_POINT
			| BGFX_TEXTURE_MIP_POINT
			| BGFX_TEXTURE_U_CLAMP
			| BGFX_TEXTURE_V_CLAMP
		);
		bgfx::TextureHandle rt[2] =
		{
			m_pickingRT,
			m_pickingRTDepth
		};
		m_pickingFB = bgfx::createFrameBuffer(BX_COUNTOF(rt), rt, true);
		return bgfx::isValid(m_pickingFB);
	}

	void AgHardwarePickingSystem::destroy()
	{
		if (bgfx::isValid(m_pickingFB))
		{
			bgfx::destroy(m_pickingFB);
		}
		if (bgfx::isValid(m_pickingRT))
		{
			bgfx::destroy(m_pickingRT);
		}
		if (bgfx::isValid(m_pickingRTDepth))
		{
			bgfx::destroy(m_pickingRTDepth);
		}
		if (bgfx::isValid(m_blitTex))
		{
			bgfx::destroy(m_blitTex);
		}
	}

	/*virtual*/
	void AgHardwarePickingSystem::setPerDrawUniforms(const AgShader* shader, const AgRenderItem* item) const
	{
		if (!shader || !item)
			return;

		// Submit ID pass based on mesh ID
		float idsF[4];
		idsF[0] = item->m_pick_id[0] / 255.0f;
		idsF[1] = item->m_pick_id[1] / 255.0f;
		idsF[2] = item->m_pick_id[2] / 255.0f;
		idsF[3] = 1.0f;

		bgfx::setUniform(shader->m_uniforms[0].uniform_handle, idsF);
	}

	void AgHardwarePickingSystem::updateView(
		bgfx::ViewId view_pass,
		float* invViewProj,
		float mouseXNDC, float mouseYNDC,
		float fov,
		float nearFrusm, float farFrusm)
	{
		bgfx::setViewFrameBuffer(view_pass, m_pickingFB);

		float pickEye[3];
		float mousePosNDC[3] = { mouseXNDC, mouseYNDC, 0.0f };
		bx::vec3MulMtxH(pickEye, mousePosNDC, invViewProj);

		float pickAt[3];
		float mousePosNDCEnd[3] = { mouseXNDC, mouseYNDC, 1.0f };
		bx::vec3MulMtxH(pickAt, mousePosNDCEnd, invViewProj);

		// Look at our unprojected point
		float pickView[16];
		bx::mtxLookAt(pickView, pickEye, pickAt);

		// Tight FOV is best for picking
		float pickProj[16];
		bx::mtxProj(pickProj, fov, 1, nearFrusm, farFrusm, m_homogeneousDepth);

		// View rect and transforms for picking pass
		bgfx::setViewRect(view_pass, 0, 0, ID_DIM, ID_DIM);
		bgfx::setViewTransform(view_pass, pickView, pickProj);

		m_isPicked = true;
	}

	uint32_t AgHardwarePickingSystem::readBlit(bgfx::ViewId view_pass)
	{
		// Blit and read
		bgfx::blit(view_pass, m_blitTex, 0, 0, m_pickingRT);
		return bgfx::readTexture(m_blitTex, m_blitData);
	}

	uint8_t AgHardwarePickingSystem::acquireResult(bool isSinglePick)
	{
		std::map<uint32_t, uint32_t> ids;  // This contains all the IDs found in the buffer
		std::pair<uint32_t, uint32_t> maxAmount = std::make_pair<uint32_t, uint32_t>(0, 0);
		for (uint8_t *x = m_blitData; x < m_blitData + ID_DIM * ID_DIM * 4;)
		{
			uint8_t rr = *x++;
			uint8_t gg = *x++;
			uint8_t bb = *x++;
			uint8_t aa = *x++;

			if (m_isDX9)
			{
				// Comes back as BGRA
				uint8_t temp = rr;
				rr = bb;
				bb = temp;
			}

			if (0 == (rr | gg | bb)) // Skip background
			{
				continue;
			}

			uint32_t hashKey = rr + (gg << 8) + (bb << 16) + (aa << 24);
			std::map<uint32_t, uint32_t>::iterator mapIter = ids.find(hashKey);
			uint32_t amount = 1;
			if (mapIter != ids.end())
			{
				amount = mapIter->second + 1;
			}

			ids[hashKey] = amount; // Amount of times this ID (color) has been clicked on in buffer
			if (amount > maxAmount.first)
			{
				maxAmount.first = amount;
				maxAmount.second = hashKey;
			}
		}

		AgSceneDatabase& scene = Singleton<AgSceneDatabase>::instance();
		scene.m_select_result.clear();

		if (0 == maxAmount.first)
			return 0;

		if (isSinglePick)
		{
			AgSceneDatabase::SelectMap::const_iterator iter = scene.m_select_id_map.find(maxAmount.second);
			if (iter != scene.m_select_id_map.end())
			{
				scene.m_select_result.push_back(iter->second);
				return 1;
			}
		}
		else
		{
			uint8_t nPickNum = 0;
			for (std::map<uint32_t, uint32_t>::iterator mapIter = ids.begin(); mapIter != ids.end(); mapIter++)
			{
				if (mapIter->second > PICKING_RESULT_THRESHOULD)
				{
					AgSceneDatabase::SelectMap::const_iterator iter = scene.m_select_id_map.find(mapIter->first);
					if (iter != scene.m_select_id_map.end())
					{
						scene.m_select_result.push_back(iter->second);
						nPickNum++;
					}
				}
			}
			return nPickNum;
		}
		return 0;
	}
}