#pragma once

namespace ambergris {

	struct SceneImportOptions {
		SceneImportOptions() : m_max_uv_channels(4), m_pack_normal(false), m_pack_uv(false) {};

		int m_max_uv_channels;
		bool m_pack_normal;
		bool m_pack_uv;
	};
}