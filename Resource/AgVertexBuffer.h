#pragma once

#include "AgResource.h"
#include <bgfx/bgfx.h>

#include "Foundation/TBuffer.h"

namespace ambergris {

#pragma pack(4)
	struct AgVertexBuffer : public AgResource
	{
		AgVertexBuffer(){
		};

		bgfx::VertexDecl		m_decl;
		TBuffer<uint8_t>		m_vertex_buffer;
	};
#pragma  pack()
}