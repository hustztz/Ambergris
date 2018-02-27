#pragma once

#include "AgObject.h"
#include "Resource/AgShader.h"
#include <tinystl/vector.h>

namespace ambergris {

	struct AgMesh : public AgObject
	{
		AgMesh() : AgObject(), m_inst_handle(-1), m_bShadowCaster(false) {};
		void evaluateBoundingBox();

		struct Geometry
		{
			Geometry()
				: vertex_buffer_handle(AgResource::kInvalidHandle)
				, index_buffer_handle(AgResource::kInvalidHandle)
				, material_handle(AgResource::kInvalidHandle)
			{
				for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; i++)
				{
					texture_handle[i] = AgResource::kInvalidHandle;
				}
			}

			bool operator==(const Geometry& other) {
				bool ret = true;
				for(uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; i ++)
				{
					ret &= (other.texture_handle[i] == this->texture_handle[i]);
				}
				return ret &&
					(other.vertex_buffer_handle == this->vertex_buffer_handle) &&
					(other.index_buffer_handle == this->index_buffer_handle) &&
					(other.material_handle == this->material_handle);
			}

			Handle	vertex_buffer_handle;
			Handle	index_buffer_handle;
			Handle	material_handle;
			Handle	texture_handle[AgShader::MAX_TEXTURE_SLOT_COUNT];
		};
		typedef stl::vector<Geometry> Geometries;
		Geometries		m_geometries;
		int				m_inst_handle;
		bool			m_bShadowCaster;		// Initial shadow preference (overridden by renderables section of .unit)
	};
}