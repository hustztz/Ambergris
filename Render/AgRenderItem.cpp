#include "AgRenderItem.h"
#include "Resource/AgRenderResourceManager.h"

#include <memory.h>

namespace ambergris {

	AgRenderItem::AgRenderItem()
		: m_vbh(BGFX_INVALID_HANDLE)
		, m_ibh(BGFX_INVALID_HANDLE)
		, m_oqh(BGFX_INVALID_HANDLE)
		, m_bbox(AgBoundingbox::kInvalidHandle)
		, m_material_handle(AgMaterial::MaterialType::E_LAMBERT)
		, m_occlusion_threshold(5)
	{
	}

	AgRenderItem::~AgRenderItem()
	{
		destroyBuffers();
	}

	void AgRenderItem::destroyBuffers()
	{
		disableOcclusionQuery();
		if (bgfx::isValid(m_vbh))
		{
			bgfx::destroy(m_vbh);
			m_vbh = BGFX_INVALID_HANDLE;
		}
		if (bgfx::isValid(m_ibh))
		{
			bgfx::destroy(m_ibh);
			m_ibh.idx = BGFX_INVALID_HANDLE;
		}
	}

	void AgRenderItem::setBuffers(
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		_ResetTransform();
		_SetVertexBuffer(decl, vertBuf, vertSize);
		_SetIndexBuffer(indexBuf, indexSize);
	}

	void AgRenderItem::_SetVertexBuffer(const bgfx::VertexDecl& decl, const uint8_t* buffer, uint32_t size)
	{
		if (!buffer || 0 == size)
			return;
		/*const bgfx::Memory* mem = bgfx::alloc(size);
		m_vbh = bgfx::createVertexBuffer(mem, decl);
		memcpy_s(mem->data, mem->size, buffer, size);*/
		const bgfx::Memory* mem = bgfx::makeRef(buffer, size);
		m_vbh = bgfx::createVertexBuffer(mem, decl);
	}

	void AgRenderItem::_SetIndexBuffer(const uint16_t* buffer, uint32_t size)
	{
		if (!buffer || 0 == size)
			return;
		/*const bgfx::Memory* mem = bgfx::alloc(size);
		m_ibh = bgfx::createIndexBuffer(mem);
		memcpy_s(mem->data, mem->size, buffer, size);*/
		const bgfx::Memory* mem = bgfx::makeRef(buffer, size/sizeof(uint16_t));
		m_ibh = bgfx::createIndexBuffer(mem);
	}

	void AgRenderItem::submit() const
	{
		bgfx::setTransform(m_mtx);
		const AgMaterial* mat = Singleton<AgRenderResourceManager>::instance().m_materials.get(m_material_handle);
		if (!mat)
			return;
		//TODO

		bgfx::setIndexBuffer(m_ibh);
		bgfx::setVertexBuffer(0, m_vbh);
	}

	void AgRenderItem::setTransform(const float* mtx)
	{
		if(mtx)
			memcpy_s(m_mtx, 16 * sizeof(float), mtx, 16 * sizeof(float));
		else
		{
			_ResetTransform();
		}
	}

	void AgRenderItem::setPickID(const uint32_t* id)
	{
		if (!id)
			return;

		memcpy_s(m_pick_id, sizeof(uint32_t)*3, id, sizeof(uint32_t) * 3);
	}

	void AgRenderItem::_ResetTransform()
	{
		m_mtx[0] = m_mtx[5] = m_mtx[9] = m_mtx[15] = 1.0f;
		m_mtx[1] = m_mtx[2] = m_mtx[3] = m_mtx[4] =
			m_mtx[6] = m_mtx[7] = m_mtx[8] = m_mtx[10] =
			m_mtx[11] = m_mtx[12] = m_mtx[13] = m_mtx[14] = 0.0f;
	}


	void AgRenderItem::enableOcclusionQuery()
	{
		m_oqh = bgfx::createOcclusionQuery();
	}

	void AgRenderItem::disableOcclusionQuery()
	{
		if (bgfx::isValid(m_oqh))
		{
			bgfx::destroy(m_oqh);
			m_oqh = BGFX_INVALID_HANDLE;
		}
	}

}
