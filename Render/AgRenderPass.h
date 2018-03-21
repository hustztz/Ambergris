#pragma once

#include <string.h>

namespace ambergris {

	struct AgRenderPass
	{
		enum RenderViewName
		{
			E_VIEW_MAIN = 0,
			E_VIEW_ID,
			E_VIEW_BLIT,
			E_VIEW_SECOND,
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
	public:
		void init(bgfx::ViewId view_id)
		{
			if (view_id >= E_VIEW_COUNT)
				return;
			if (m_pass_state[view_id].isValid)
				return;

			m_pass_state[view_id].isValid = true;

			switch (view_id)
			{
			case E_VIEW_ID:
				// ID buffer clears to black, which represnts clicking on nothing (background)
				bgfx::setViewClear(E_VIEW_ID
					, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
					, 0x000000ff
					, 1.0f
					, 0
				);
				break;
			case E_VIEW_SECOND:
				// ID buffer clears to black, which represnts clicking on nothing (background)
				bgfx::setViewClear(E_VIEW_ID
					, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
					, 0x202040ff
					, 1.0f
					, 0
				);
				break;
			default:
				bgfx::setViewClear(view_id
					, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
					, 0x303030ff
					, 1.0f
					, 0
				);
				break;
			}
		}

		void clear()
		{
			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			for (uint16_t i = 0; i < E_VIEW_COUNT; i++)
			{
				if (!m_pass_state[i].isValid)
					continue;

				switch (i)
				{
				case E_VIEW_MAIN:
					// Set view 0 default viewport.
					bgfx::setViewRect(E_VIEW_MAIN, 0, 0, bgfx::BackbufferRatio::Equal);
					// This dummy draw call is here to make sure that view 0 is cleared
					// if no other draw calls are submitted to view 0.
					bgfx::touch(E_VIEW_MAIN);
					break;
				case E_VIEW_SECOND:
					// Set view 0 default viewport.
					bgfx::setViewRect(E_VIEW_SECOND, 10, uint16_t(m_height - m_height / 4 - 10), uint16_t(m_width / 4), uint16_t(m_height / 4));
					bgfx::touch(E_VIEW_SECOND);
					break;
				default:
					break;
				}
			}
		}
		AgRenderPass() {}
		~AgRenderPass(){}
	public:
		struct RenderPassState
		{
			RenderPassState()
				: isValid(false) {}

			bool			isValid;
		};

		RenderPassState	m_pass_state[E_VIEW_COUNT];
		uint32_t		m_width;
		uint32_t		m_height;
	};
}
