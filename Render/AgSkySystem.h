#pragma once
#include "AgFxSystem.h"
#include "AgDynamicValueController.h"
#include "Resource/AgShader.h"
#include "Resource/AgTexture.h"

namespace ambergris {

	class AgSkySystem : public AgFxSystem
	{
		// Controls sun position according to time, month, and observer's latitude.
		// Sun position computation based on Earth's orbital elements: https://nssdc.gsfc.nasa.gov/planetary/factsheet/earthfact.html
		class SunController
		{
		public:
			enum Month : int
			{
				January = 0,
				February,
				March,
				April,
				May,
				June,
				July,
				August,
				September,
				October,
				November,
				December
			};

			SunController();

			void update(float time);

			float m_northDirection[3];
			float m_sunDirection[4];
			float m_upvector[3];
			float m_latitude;
			Month m_month;
		private:
			void _CalculateSunOrbit();
			void _UpdateSunPosition(float hour);

			float m_eclipticObliquity;
			float m_delta;
		};
		// Renders a screen-space grid of triangles.
		// Because of performance reasons, and because sky color is smooth, sky color is computed in vertex shader.
		// 32x32 is a reasonable size for the grid to have smooth enough colors.
		struct ProceduralSky
		{
			void init(int verticalCount, int horizontalCount);
			void destroy();
			void draw(float sunDirection[],
				const AgDynamicValueController::Color& skyLuminanceXYZ,
				float exposition[],
				float turbidity,
				const AgDynamicValueController::Color& sunLuminanceRGB) const;

			bgfx::VertexBufferHandle m_vbh;
			bgfx::IndexBufferHandle m_ibh;

			AgShader::Handle		m_skyShader;
			AgShader::Handle		m_skyShader_colorBandingFix;

			bool m_preventBanding;
		};
	public:
		AgSkySystem();
		virtual ~AgSkySystem();

		virtual bool init() override;
		virtual void destroy() override;
		virtual void auxiliaryDraw() override;
		virtual void setOverrideResource(const AgShader* shader, void* data) const override;
		virtual void updateTime(float time) override;
		virtual AgShader::Handle getOverrideShader() const override { return AgShader::E_SKY_LANDSCAPE_SHADER; }
	private:
		ProceduralSky m_sky;
		SunController m_sun;

		AgDynamicValueController m_sunLuminanceXYZ;
		AgDynamicValueController m_skyLuminanceXYZ;

		float m_time;
		float m_turbidity;
		AgTexture::Handle m_lightmapTexture;
	};

}
