#pragma once

#include "Foundation/Singleton.h"
#include "BgfxNodeHeirarchy.h"

#include <boost/ptr_container/ptr_vector.hpp>

namespace ambergris {

	struct BgfxSceneDatabase
	{
	public:
		void Reset();
		void AppendNode(BgfxNodeHeirarchy* node);
		size_t GetNodeSize() const { return m_node_arr.size(); }
		const BgfxNodeHeirarchy& GetNode(int id) const { return m_node_arr[id]; }
	private:
		BgfxSceneDatabase() {};
		~BgfxSceneDatabase() { Reset(); }
		friend class Singleton<BgfxSceneDatabase>;
	private:
		boost::ptr_vector<BgfxNodeHeirarchy>	m_node_arr;
	};
}