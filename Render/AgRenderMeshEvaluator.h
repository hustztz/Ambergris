#pragma once
#include "AgRenderSceneBridge.h"
#include "Resource\AgRenderResourceManager.h"
#include "Scene\AgMesh.h"
#include "AgRenderer.h"

#include "BGFX/entry/entry.h" //TODO

namespace ambergris {

	class AgRenderMeshEvaluator : public AgRenderEvaluator
	{
	public:
		AgRenderMeshEvaluator()
			: AgRenderEvaluator()	{}
		~AgRenderMeshEvaluator() {}

		virtual bool evaluate(AgRenderer& renderer, const AgObject* pObject) override;

	protected:
		template<typename T>
		static bool _AppendRenderNode(AgRenderer& renderer, AgGeometry::Handle geom, const AgMesh* mesh)
		{
			if (!mesh)
				return false;

			const AgGeometry* pGeometry = Singleton<AgRenderResourceManager>::instance().m_geometries.get(geom);
			if (!pGeometry)
				return false;

			const AgVertexBuffer* vb = Singleton<AgRenderResourceManager>::instance().m_vertex_buffer_pool.get(pGeometry->vertex_buffer_handle);
			const AgIndexBuffer* ib = Singleton<AgRenderResourceManager>::instance().m_index_buffer_pool.get(pGeometry->index_buffer_handle);
			if (!vb || !ib)
				return false;

			std::shared_ptr<T> renderNode(BX_NEW(entry::getAllocator(), T));
			for (uint8_t tex_stage = 0; tex_stage < AgShader::MAX_TEXTURE_SLOT_COUNT; tex_stage++)
			{
				AgTexture::Handle texture_handle = pGeometry->texture_handle[tex_stage];
				if (AgTexture::kInvalidHandle == texture_handle)
					break;
				renderNode->setTexture(tex_stage, texture_handle);
			}
			if (!renderNode->appendGeometry(
				mesh->m_global_transform,
				pGeometry->material_handle,
				mesh->m_pick_id,
				vb->m_decl,
				vb->m_vertex_buffer.GetData(), vb->m_vertex_buffer.GetSize() * sizeof(uint8_t),
				ib->m_index_buffer.GetData(), ib->m_index_buffer.GetSize() * sizeof(uint16_t)))
			{
				return false;
			}
			return nullptr != renderer.appendNode(renderNode, geom);
		}

		template<typename T>
		static bool _EvaluateMeshImpl(AgRenderer& renderer, const AgMesh* mesh)
		{
			if (!mesh)
				return false;

			const int nSubMeshNum = (const int)mesh->m_geometries.size();
			if (0 == nSubMeshNum)
				return false;

			for (int i = 0; i < nSubMeshNum; i++)
			{
				_AppendRenderNode<T>(renderer, mesh->m_geometries.at(i), mesh);
			}
			return true;
		}
	};
}
