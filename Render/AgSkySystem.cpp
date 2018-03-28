#include "AgSkySystem.h"
#include "Resource/AgRenderResourceManager.h"
#include "Resource/AgShaderUtils.h"

#include <bx/math.h>

#include "BGFX/entry/entry.h"//TODO

namespace ambergris {

	AgSkySystem::SunController::SunController() :
		m_latitude(50.0f),
		m_month(June),
		m_eclipticObliquity(bx::toRad(23.4f)),
		m_delta(0.0f)
	{
		m_northDirection[0] = 1.0;
		m_northDirection[1] = 0.0;
		m_northDirection[2] = 0.0;
		m_upvector[0] = 0.0f;
		m_upvector[1] = 1.0f;
		m_upvector[2] = 0.0f;
	}

	void AgSkySystem::SunController::update(float time)
	{
		_CalculateSunOrbit();
		_UpdateSunPosition(time - 12.0f);
	}

	void AgSkySystem::SunController::_CalculateSunOrbit()
	{
		float day = 30.0f * m_month + 15.0f;
		float lambda = 280.46f + 0.9856474f * day;
		lambda = bx::toRad(lambda);
		m_delta = bx::asin(bx::sin(m_eclipticObliquity) * bx::sin(lambda));
	}

	void AgSkySystem::SunController::_UpdateSunPosition(float hour)
	{
		float latitude = bx::toRad(m_latitude);
		float h = hour * bx::kPi / 12.0f;
		float azimuth = bx::atan2(
			bx::sin(h),
			bx::cos(h) * bx::sin(latitude) - bx::tan(m_delta) * bx::cos(latitude)
		);

		float altitude = bx::asin(
			bx::sin(latitude) * bx::sin(m_delta) + bx::cos(latitude) * bx::cos(m_delta) * bx::cos(h)
		);
		float rotation[4];
		bx::quatRotateAxis(rotation, m_upvector, -azimuth);
		float direction[3];
		bx::vec3MulQuat(direction, m_northDirection, rotation);
		float v[3];
		bx::vec3Cross(v, m_upvector, direction);
		bx::quatRotateAxis(rotation, v, altitude);
		bx::vec3MulQuat(m_sunDirection, direction, rotation);
	}


	struct ScreenPosVertex
	{
		float m_x;
		float m_y;

		static void init()
		{
			ms_decl
				.begin()
				.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
				.end();
		}

		static bgfx::VertexDecl ms_decl;
	};
	bgfx::VertexDecl ScreenPosVertex::ms_decl;

	void AgSkySystem::ProceduralSky::init(int verticalCount, int horizontalCount)
	{
		// Create vertex stream declaration.
		ScreenPosVertex::init();

		m_preventBanding = true;

		bx::AllocatorI* allocator = entry::getAllocator();

		ScreenPosVertex* vertices = (ScreenPosVertex*)BX_ALLOC(allocator
			, verticalCount * horizontalCount * sizeof(ScreenPosVertex)
		);

		for (int i = 0; i < verticalCount; i++)
		{
			for (int j = 0; j < horizontalCount; j++)
			{
				ScreenPosVertex& v = vertices[i * verticalCount + j];
				v.m_x = float(j) / (horizontalCount - 1) * 2.0f - 1.0f;
				v.m_y = float(i) / (verticalCount - 1) * 2.0f - 1.0f;
			}
		}

		uint16_t* indices = (uint16_t*)BX_ALLOC(allocator
			, (verticalCount - 1) * (horizontalCount - 1) * 6 * sizeof(uint16_t)
		);

		int k = 0;
		for (int i = 0; i < verticalCount - 1; i++)
		{
			for (int j = 0; j < horizontalCount - 1; j++)
			{
				indices[k++] = (uint16_t)(j + 0 + horizontalCount * (i + 0));
				indices[k++] = (uint16_t)(j + 1 + horizontalCount * (i + 0));
				indices[k++] = (uint16_t)(j + 0 + horizontalCount * (i + 1));

				indices[k++] = (uint16_t)(j + 1 + horizontalCount * (i + 0));
				indices[k++] = (uint16_t)(j + 1 + horizontalCount * (i + 1));
				indices[k++] = (uint16_t)(j + 0 + horizontalCount * (i + 1));
			}
		}

		m_vbh = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(ScreenPosVertex) * verticalCount * horizontalCount), ScreenPosVertex::ms_decl);
		m_ibh = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(uint16_t) * k));

		BX_FREE(allocator, indices);
		BX_FREE(allocator, vertices);
	}

	void AgSkySystem::ProceduralSky::destroy()
	{
		bgfx::destroy(m_ibh);
		bgfx::destroy(m_vbh);
	}

	// Turbidity tables. Taken from:
	// A. J. Preetham, P. Shirley, and B. Smits. A Practical Analytic Model for Daylight. SIGGRAPH ¡¯99
	// Coefficients correspond to xyY colorspace.
	static AgDynamicValueController::Color ABCDE[] =
	{
		{ { -0.2592f, -0.2608f, -1.4630f } },
		{ { 0.0008f,  0.0092f,  0.4275f } },
		{ { 0.2125f,  0.2102f,  5.3251f } },
		{ { -0.8989f, -1.6537f, -2.5771f } },
		{ { 0.0452f,  0.0529f,  0.3703f } }
	};
	static AgDynamicValueController::Color ABCDE_t[] =
	{
		{ { -0.0193f, -0.0167f,  0.1787f } },
		{ { -0.0665f, -0.0950f, -0.3554f } },
		{ { -0.0004f, -0.0079f, -0.0227f } },
		{ { -0.0641f, -0.0441f,  0.1206f } },
		{ { -0.0033f, -0.0109f, -0.0670f } }
	};

	void computePerezCoeff(float turbidity, float* perezCoeff)
	{
		for (int i = 0; i < 5; ++i)
		{
			AgDynamicValueController::Color tmp;
			bx::vec3Mul(tmp.data, ABCDE_t[i].data, turbidity);
			bx::vec3Add(perezCoeff + 4 * i, tmp.data, ABCDE[i].data);
			perezCoeff[4 * i + 3] = 0.0f;
		}
	}

	void AgSkySystem::ProceduralSky::draw(
		float sunDirection[],
		const AgDynamicValueController::Color& skyLuminanceXYZ,
		float exposition[],
		float turbidity,
		const AgDynamicValueController::Color& sunLuminanceRGB
	) const
	{
		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(m_preventBanding ? AgShader::E_SKY_COLOR_BANDING : AgShader::E_SKY);
		if (!shader)
			return;

		bgfx::setUniform(shader->m_uniforms[0].uniform_handle, sunDirection);
		bgfx::setUniform(shader->m_uniforms[1].uniform_handle, skyLuminanceXYZ.data);
		bgfx::setUniform(shader->m_uniforms[2].uniform_handle, exposition);

		float perezCoeff[4 * 5];
		computePerezCoeff(turbidity, perezCoeff);
		bgfx::setUniform(shader->m_uniforms[3].uniform_handle, perezCoeff);
		bgfx::setUniform(shader->m_uniforms[4].uniform_handle, sunLuminanceRGB.data);

		bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_EQUAL);
		bgfx::setIndexBuffer(m_ibh);
		bgfx::setVertexBuffer(0, m_vbh);
		
		bgfx::submit(0, shader->m_program);
	}


	AgSkySystem::AgSkySystem()
		: m_turbidity(2.15f)
		, m_time(0.0f)
		, m_lightmapTexture(AgTexture::kInvalidHandle)
	{
	}

	AgSkySystem::~AgSkySystem()
	{
		destroy();
	}

	// Precomputed luminance of sunlight in XYZ colorspace.
	// Computed using code from Game Engine Gems, Volume One, chapter 15. Implementation based on Dr. Richard Bird model.
	// This table is used for piecewise linear interpolation. Transitions from and to 0.0 at sunset and sunrise are highly inaccurate
	static std::map<float, AgDynamicValueController::Color> sunLuminanceXYZTable = {
		{ 5.0f,{ { 0.000000f, 0.000000f, 0.000000f } } },
		{ 7.0f,{ { 12.703322f, 12.989393f, 9.100411f } } },
		{ 8.0f,{ { 13.202644f, 13.597814f, 11.524929f } } },
		{ 9.0f,{ { 13.192974f, 13.597458f, 12.264488f } } },
		{ 10.0f,{ { 13.132943f, 13.535914f, 12.560032f } } },
		{ 11.0f,{ { 13.088722f, 13.489535f, 12.692996f } } },
		{ 12.0f,{ { 13.067827f, 13.467483f, 12.745179f } } },
		{ 13.0f,{ { 13.069653f, 13.469413f, 12.740822f } } },
		{ 14.0f,{ { 13.094319f, 13.495428f, 12.678066f } } },
		{ 15.0f,{ { 13.142133f, 13.545483f, 12.526785f } } },
		{ 16.0f,{ { 13.201734f, 13.606017f, 12.188001f } } },
		{ 17.0f,{ { 13.182774f, 13.572725f, 11.311157f } } },
		{ 18.0f,{ { 12.448635f, 12.672520f, 8.267771f } } },
		{ 20.0f,{ { 0.000000f, 0.000000f, 0.000000f } } }
	};


	// Precomputed luminance of sky in the zenith point in XYZ colorspace.
	// Computed using code from Game Engine Gems, Volume One, chapter 15. Implementation based on Dr. Richard Bird model.
	// This table is used for piecewise linear interpolation. Day/night transitions are highly inaccurate.
	// The scale of luminance change in Day/night transitions is not preserved.
	// Luminance at night was increased to eliminate need the of HDR render.
	static std::map<float, AgDynamicValueController::Color> skyLuminanceXYZTable = {
		{ 0.0f,{ { 0.308f, 0.308f, 0.411f } } },
		{ 1.0f,{ { 0.308f, 0.308f, 0.410f } } },
		{ 2.0f,{ { 0.301f, 0.301f, 0.402f } } },
		{ 3.0f,{ { 0.287f, 0.287f, 0.382f } } },
		{ 4.0f,{ { 0.258f, 0.258f, 0.344f } } },
		{ 5.0f,{ { 0.258f, 0.258f, 0.344f } } },
		{ 7.0f,{ { 0.962851f, 1.000000f, 1.747835f } } },
		{ 8.0f,{ { 0.967787f, 1.000000f, 1.776762f } } },
		{ 9.0f,{ { 0.970173f, 1.000000f, 1.788413f } } },
		{ 10.0f,{ { 0.971431f, 1.000000f, 1.794102f } } },
		{ 11.0f,{ { 0.972099f, 1.000000f, 1.797096f } } },
		{ 12.0f,{ { 0.972385f, 1.000000f, 1.798389f } } },
		{ 13.0f,{ { 0.972361f, 1.000000f, 1.798278f } } },
		{ 14.0f,{ { 0.972020f, 1.000000f, 1.796740f } } },
		{ 15.0f,{ { 0.971275f, 1.000000f, 1.793407f } } },
		{ 16.0f,{ { 0.969885f, 1.000000f, 1.787078f } } },
		{ 17.0f,{ { 0.967216f, 1.000000f, 1.773758f } } },
		{ 18.0f,{ { 0.961668f, 1.000000f, 1.739891f } } },
		{ 20.0f,{ { 0.264f, 0.264f, 0.352f } } },
		{ 21.0f,{ { 0.264f, 0.264f, 0.352f } } },
		{ 22.0f,{ { 0.290f, 0.290f, 0.386f } } },
		{ 23.0f,{ { 0.303f, 0.303f, 0.404f } } }
	};

	bool AgSkySystem::_PrepareShader()
	{
		{
			AgShader* landscape_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SKY_LANDSCAPE_SHADER);
			if (!landscape_shader)
				return false;

			if (AgShader::kInvalidHandle == landscape_shader->m_handle)
			{
				landscape_shader->m_program = shaderUtils::loadProgram("vs_mesh", "fs_sky_landscape");
				if (!bgfx::isValid(landscape_shader->m_program))
					return false;

				landscape_shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
				landscape_shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;
				landscape_shader->m_texture_slots[1].uniform_handle = bgfx::createUniform("s_texLightmap", bgfx::UniformType::Int1);
				landscape_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
				landscape_shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_sunLuminance", bgfx::UniformType::Vec4);
				landscape_shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_skyLuminance", bgfx::UniformType::Vec4);
				landscape_shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_parameters", bgfx::UniformType::Vec4);

				landscape_shader->m_handle = AgShader::E_SKY_LANDSCAPE_SHADER;
			}
		}
		
		{
			AgShader* landscape_instance_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SKY_LANDSCAPE_INSTANCE_SHADER);
			if (!landscape_instance_shader)
				return false;

			if (AgShader::kInvalidHandle == landscape_instance_shader->m_handle)
			{
				landscape_instance_shader->m_program = shaderUtils::loadProgram("vs_instancing", "fs_sky_landscape");
				if (!bgfx::isValid(landscape_instance_shader->m_program))
					return false;

				landscape_instance_shader->m_texture_slots[0].uniform_handle = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
				landscape_instance_shader->m_texture_slots[0].texture_state = AMBERGRIS_TEXTURE_STATE_FILTER;
				landscape_instance_shader->m_texture_slots[1].uniform_handle = bgfx::createUniform("s_texLightmap", bgfx::UniformType::Int1);
				landscape_instance_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
				landscape_instance_shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_sunLuminance", bgfx::UniformType::Vec4);
				landscape_instance_shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_skyLuminance", bgfx::UniformType::Vec4);
				landscape_instance_shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_parameters", bgfx::UniformType::Vec4);

				landscape_instance_shader->m_handle = AgShader::E_SKY_LANDSCAPE_INSTANCE_SHADER;
			}
		}
		
		{
			AgShader* sky_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SKY);
			if (!sky_shader)
				return false;

			if (AgShader::kInvalidHandle == sky_shader->m_handle)
			{
				sky_shader->m_program = shaderUtils::loadProgram("vs_sky", "fs_sky");
				if (!bgfx::isValid(sky_shader->m_program))
					return false;

				sky_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
				sky_shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_skyLuminanceXYZ", bgfx::UniformType::Vec4);
				sky_shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_parameters", bgfx::UniformType::Vec4);
				sky_shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_perezCoeff", bgfx::UniformType::Vec4, 5);
				sky_shader->m_uniforms[4].uniform_handle = bgfx::createUniform("u_sunLuminance", bgfx::UniformType::Vec4);

				sky_shader->m_handle = AgShader::E_SKY;
			}
		}
		
		{
			AgShader* color_sky_shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(AgShader::E_SKY_COLOR_BANDING);
			if (!color_sky_shader)
				return false;

			if (AgShader::kInvalidHandle == color_sky_shader->m_handle)
			{
				color_sky_shader->m_program = shaderUtils::loadProgram("vs_sky", "fs_sky_color_banding_fix");
				if (!bgfx::isValid(color_sky_shader->m_program))
					return false;

				color_sky_shader->m_uniforms[0].uniform_handle = bgfx::createUniform("u_sunDirection", bgfx::UniformType::Vec4);
				color_sky_shader->m_uniforms[1].uniform_handle = bgfx::createUniform("u_skyLuminanceXYZ", bgfx::UniformType::Vec4);
				color_sky_shader->m_uniforms[2].uniform_handle = bgfx::createUniform("u_parameters", bgfx::UniformType::Vec4);
				color_sky_shader->m_uniforms[3].uniform_handle = bgfx::createUniform("u_perezCoeff", bgfx::UniformType::Vec4, 5);
				color_sky_shader->m_uniforms[4].uniform_handle = bgfx::createUniform("u_sunLuminance", bgfx::UniformType::Vec4);

				color_sky_shader->m_handle = AgShader::E_SKY_COLOR_BANDING;
			}
		}
		
		return true;
	}

	bool AgSkySystem::init()
	{
		if (!_PrepareShader())
		{
			printf("Failed to load sky shaders.\n");
			return false;
		}

		m_sunLuminanceXYZ.SetMap(sunLuminanceXYZTable);
		m_skyLuminanceXYZ.SetMap(skyLuminanceXYZTable);
		m_time = 0.0f;
		m_sky.init(32, 32);
		m_sun.update(0);
		m_lightmapTexture = Singleton<AgRenderResourceManager>::instance().m_textures.find("textures/lightmap.ktx");
		if (AgTexture::kInvalidHandle == m_lightmapTexture)
		{
			// TODO
			entry::setCurrentDir("runtime/");
			m_lightmapTexture = Singleton<AgRenderResourceManager>::instance().m_textures.append("textures/lightmap.ktx", 0);
		}
		return (AgTexture::kInvalidHandle != m_lightmapTexture);
	}

	void AgSkySystem::destroy()
	{
		m_sky.destroy();
	}

	/*virtual*/
	void AgSkySystem::begin()
	{
		_SetPerFrameUniforms();
	}

	/*virtual*/
	void AgSkySystem::end()
	{
		AgDynamicValueController::Color sunLuminanceXYZ = m_sunLuminanceXYZ.GetValue(m_time);
		AgDynamicValueController::Color sunLuminanceRGB = AgDynamicValueController::XYZToRGB(sunLuminanceXYZ);
		AgDynamicValueController::Color skyLuminanceXYZ = m_skyLuminanceXYZ.GetValue(m_time);

		float exposition[4] = { 0.02f, 3.0f, 0.1f, m_time };

		m_sky.draw(m_sun.m_sunDirection,
			skyLuminanceXYZ,
			exposition,
			m_turbidity,
			sunLuminanceRGB);
	}

	/*virtual*/
	void AgSkySystem::updateTime(float time)
	{
		m_time = bx::mod(time, 24.0f);
		m_sun.update(m_time);
	}

	void AgSkySystem::_SetPerFrameUniforms() const
	{
		const AgShader* shader = Singleton<AgRenderResourceManager>::instance().m_shaders.get(getOverrideShader());
		if (!shader)
			return;

		bgfx::setUniform(shader->m_uniforms[0].uniform_handle, m_sun.m_sunDirection);
		AgDynamicValueController::Color sunLuminanceXYZ = m_sunLuminanceXYZ.GetValue(m_time);
		AgDynamicValueController::Color sunLuminanceRGB = AgDynamicValueController::XYZToRGB(sunLuminanceXYZ);
		bgfx::setUniform(shader->m_uniforms[1].uniform_handle, sunLuminanceRGB.data);
		AgDynamicValueController::Color skyLuminanceXYZ = m_skyLuminanceXYZ.GetValue(m_time);
		AgDynamicValueController::Color skyLuminanceRGB = AgDynamicValueController::XYZToRGB(skyLuminanceXYZ);
		bgfx::setUniform(shader->m_uniforms[2].uniform_handle, skyLuminanceRGB.data);
		float exposition[4] = { 0.02f, 3.0f, 0.1f, m_time };
		bgfx::setUniform(shader->m_uniforms[3].uniform_handle, exposition);

		const AgTexture* tex = Singleton<AgRenderResourceManager>::instance().m_textures.get(m_lightmapTexture);
		if (tex)
		{
			uint8_t textureStage = 1;
			bgfx::setTexture(textureStage, shader->m_texture_slots[textureStage].uniform_handle, tex->m_tex_handle, shader->m_texture_slots[textureStage].texture_state);
		}
	}

}