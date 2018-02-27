#pragma once
#include "AgRenderSceneBridge.h"
#include "AgRenderNode.h"
#include "Resource\AgGeometryResourceManager.h"
#include "Scene\AgMesh.h"

namespace ambergris {

	template<typename T>
	class AgRenderMeshEvaluator : public AgRenderBaseEvaluator
	{
	public:
		AgRenderMeshEvaluator(EvaluateNodeArr& nodes) : AgRenderBaseEvaluator(nodes)	{}
		virtual ~AgRenderMeshEvaluator() {}

		virtual bool evaluate(const AgMesh& geomNode) override
		{
			const int nSubMeshNum = (const int)geomNode.m_geometries.size();
			if (0 == nSubMeshNum)
				return false;

			for (int i = 0; i < nSubMeshNum; i ++)
			{
				const AgVertexBuffer* vb = Singleton<AgGeometryResourceManager>::instance().m_vertex_buffer_pool.get(geomNode.m_geometries[i].vertex_buffer_handle);
				const AgIndexBuffer* ib = Singleton<AgGeometryResourceManager>::instance().m_index_buffer_pool.get(geomNode.m_geometries[i].index_buffer_handle);
				if (!vb || !ib)
					continue;

				std::shared_ptr<T> renderNode(new T());
				renderNode->setMaterial(geomNode.m_geometries[i].material_handle);
				for (uint8_t tex_stage = 0; tex_stage < AgShader::MAX_TEXTURE_SLOT_COUNT; tex_stage ++)
				{
					AgTexture::Handle texture_handle = geomNode.m_geometries[i].texture_handle[tex_stage];
					if(AgTexture::kInvalidHandle == texture_handle)
						break;
					renderNode->setTexture(tex_stage, texture_handle);
				}
				if (!renderNode->appendGeometry(
					geomNode.m_global_transform,
					vb->m_decl,
					vb->m_vertex_buffer.GetData(), vb->m_vertex_buffer.GetSize() * sizeof(uint8_t),
					ib->m_index_buffer.GetData(), ib->m_index_buffer.GetSize() * sizeof(uint16_t)))
				{
					return false;
				}
				m_evaluate_nodes.push_back(EvaluateNode(geomNode.m_geometries[i], renderNode));
			}
			return true;
		}
	};
}
