#pragma once

namespace ambergris {

	struct AgSceneImportOptions {
		AgSceneImportOptions() : m_max_uv_channels(1), m_pack_normal(false), m_pack_uv(false) {};

		int m_max_uv_channels;//TODO
		bool m_pack_normal;
		bool m_pack_uv;
	};
}