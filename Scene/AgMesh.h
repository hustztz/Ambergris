#pragma once

#include "AgObject.h"
#include "Resource/AgGeometry.h"

#ifdef USING_TINYSTL
#include <tinystl/vector.h>
#else
#include <vector>
#endif

#include <bx/rng.h>

namespace ambergris {

	struct AgMesh : public AgObject
	{
		AgMesh() : AgObject(), m_inst_handle(-1), m_bShadowCaster(false) {
			static bx::RngMwc mwc;  // Random number generator
			m_pick_id[0] = mwc.gen() % 256;
			m_pick_id[1] = mwc.gen() % 256;
			m_pick_id[2] = mwc.gen() % 256;
		};
		void evaluateBoundingBox(AgBoundingbox* bbox);

		
#ifdef USING_TINYSTL
		typedef stl::vector<AgGeometry::Handle> Geometries;
#else
		typedef std::vector<AgGeometry::Handle> Geometries;
#endif
		Geometries		m_geometries;
		int				m_inst_handle;
		uint32_t		m_pick_id[3];
		bool			m_bShadowCaster;		// Initial shadow preference (overridden by renderables section of .unit)
	};
}