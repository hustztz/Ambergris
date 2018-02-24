#pragma once
#include "AgResource.h"

#include "Foundation/TBuffer.h"

namespace ambergris {

#pragma pack(4)
	struct AgIndexBuffer : public AgResource
	{
		AgIndexBuffer() : m_bAllByControlPoint(false) {
		};

		bool					m_bAllByControlPoint;	// Save data in VBO by control point or by polygon vertex.
		TBuffer<uint16_t>		m_index_buffer;

	};
#pragma  pack()
}