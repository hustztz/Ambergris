#pragma once

namespace ambergris {

	struct AgRenderPass
	{
		enum RenderViewName
		{
			E_VIEW_MAIN = 0,
			E_VIEW_SECOND,
			E_VIEW_ID,
			E_VIEW_BLIT,
			E_VIEW_SHADOWMAP_0_ID,
			E_VIEW_SHADOWMAP_1_ID,
			E_VIEW_SHADOWMAP_2_ID,
			E_VIEW_SHADOWMAP_3_ID,
			E_VIEW_SHADOWMAP_4_ID,
			E_VIEW_VBLUR_0_ID,
			E_VIEW_HBLUR_0_ID,
			E_VIEW_VBLUR_1_ID,
			E_VIEW_HBLUR_1_ID,
			E_VIEW_VBLUR_2_ID,
			E_VIEW_HBLUR_2_ID,
			E_VIEW_VBLUR_3_ID,
			E_VIEW_HBLUR_3_ID,

			E_VIEW_COUNT
		};
		static const uint8_t RENDER_MAX_VIEW = E_VIEW_SECOND;
	};
}
