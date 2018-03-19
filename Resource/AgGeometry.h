#pragma once
#include "AgResourcePool.h"
#include "Resource/AgShader.h"

namespace ambergris {

	struct AgGeometry : public AgResource
	{
		AgGeometry()
			: vertex_buffer_handle(AgResource::kInvalidHandle)
			, index_buffer_handle(AgResource::kInvalidHandle)
			, material_handle(AgResource::kInvalidHandle)
		{
			for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; i++)
			{
				texture_handle[i] = AgResource::kInvalidHandle;
			}
		}

		bool operator==(const AgGeometry& other) {
			bool ret = true;
			for (uint8_t i = 0; i < AgShader::MAX_TEXTURE_SLOT_COUNT; i++)
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

	class AgGeometryManager : public AgResourcePool<AgGeometry>
	{
	};
}