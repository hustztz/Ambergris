#include "AgRenderNode.h"

#include <assert.h>

namespace ambergris {

	void AgRenderNode::DestroyGeometry()
	{
		for each (auto item in m_items)
		{
			item.DestroyBuffers();
		}
		m_items.clear();
	}

	bool AgRenderNode::AppendGeometry(
		const float* transform,
		const bgfx::VertexDecl& decl,
		const uint8_t* vertBuf, uint32_t vertSize,
		const uint16_t* indexBuf, uint32_t indexSize)
	{
		if (!vertBuf || 0 == vertSize || !indexBuf || 0 == indexSize)
			return false;

		if (m_items.empty())
			m_decl = decl;
		else
		{
			assert(decl.getStride() == m_decl.getStride());
			if (decl.getStride() != m_decl.getStride())
				return false;
		}

		m_items.push_back(AgRenderItem(
			transform,
			decl,
			vertBuf, vertSize,
			indexBuf, indexSize));
		return true;
	}

	void AgRenderNode::Draw(bgfx::ViewId view) const
	{
		uint64_t state = Singleton<AgMaterialManager>::instance().GetRenderState(m_materialID);
		bgfx::ProgramHandle progHandle = Singleton<AgMaterialManager>::instance().GetProgramHandle(m_materialID);
		bgfx::setState(state);
		for (AgRenderNode::RenderItemArray::const_iterator it = m_items.begin(), itEnd = m_items.end(); it != itEnd; ++it)
		{
			it->SubmitBuffers();
			bgfx::submit(view, progHandle, 0, it != itEnd - 1);
		}
	}
}