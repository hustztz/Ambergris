#pragma once

#include <bgfx/bgfx.h>

#define AMBERGRIS_DEFAULT_STATE (0            \
			| BGFX_STATE_WRITE_RGB       \
			| BGFX_STATE_WRITE_A     \
			| BGFX_STATE_DEPTH_TEST_LESS     \
			| BGFX_STATE_WRITE_Z     \
			| BGFX_STATE_CULL_CCW     \
			| BGFX_STATE_MSAA     \
			)

namespace ambergris {

	struct AgRenderState
	{
		AgRenderState()
			: m_state(AMBERGRIS_DEFAULT_STATE)
			, m_blendFactorRgba(UINT32_MAX)
			, m_fstencil(BGFX_STENCIL_NONE)
			, m_bstencil(BGFX_STENCIL_NONE)
		{}

		AgRenderState(uint64_t _state, uint32_t _blendFactorRgba, uint32_t _fstencil, uint32_t _bstencil)
			: m_state(_state)
			, m_blendFactorRgba(_blendFactorRgba)
			, m_fstencil(_fstencil)
			, m_bstencil(_bstencil)
		{}

		bool isDefaultState() const {
			return AMBERGRIS_DEFAULT_STATE == m_state &&
				UINT32_MAX == m_blendFactorRgba &&
				BGFX_STENCIL_NONE == m_fstencil &&
				BGFX_STENCIL_NONE == m_bstencil;
		}

		uint64_t m_state;
		uint32_t m_blendFactorRgba;
		uint32_t m_fstencil;
		uint32_t m_bstencil;
	};
}
