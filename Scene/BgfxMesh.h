#pragma once

#include <bgfx/bgfx.h>

#include "Foundation/TBuffer.h"

namespace ambergris {

#pragma pack(4)
	struct BgfxMesh
	{
		BgfxMesh() : m_bAllByControlPoint(false) {
		};

		bool					m_bAllByControlPoint;	// Save data in VBO by control point or by polygon vertex.
		bgfx::VertexDecl		m_decl;
		TBuffer<uint8_t>		m_vertex_buffer;
		TBuffer<uint16_t>		m_index_buffer;
	private:
		BgfxMesh(const BgfxMesh&);
		BgfxMesh& operator=(const BgfxMesh&);
	};
#pragma  pack()
}