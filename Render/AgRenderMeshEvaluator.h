#pragma once
#include "AgRenderSceneBridge.h"
#include "Resource\AgRenderResourceManager.h"
#include "Scene\AgMesh.h"

#include "BGFX/entry/entry.h" //TODO

namespace ambergris {

	class AgRenderMeshEvaluator : public AgRenderEvaluator
	{
	public:
		struct MeshEvaluatorInfo : public ObjectEvaluatorInfo
		{
			MeshEvaluatorInfo(std::shared_ptr<AgRenderNode> renderNode, AgObject::Handle objectHandle, const AgMesh::Geometry& geometry)
				: ObjectEvaluatorInfo(renderNode, objectHandle), m_geometry(geometry)
			{
			}
			AgMesh::Geometry				m_geometry;
		};
	public:
		AgRenderMeshEvaluator()
			: AgRenderEvaluator()	{}
		~AgRenderMeshEvaluator() {}

		virtual bool evaluate(const AgObject* pObject) override;
		virtual void bridgeRenderer(AgRenderer& renderer) const override;

	protected:
#ifdef USING_TINYSTL
		typedef  stl::vector<std::shared_ptr<ObjectEvaluatorInfo>> EvaluateInfoArr;
#else
		typedef  std::vector<std::shared_ptr<ObjectEvaluatorInfo>> EvaluateInfoArr;
#endif
		EvaluateInfoArr			m_evaluate_mapping;

	protected:
		template<typename T>
		static bool _AppendRenderNode(int subId, const AgMesh* mesh, EvaluateInfoArr& evaluatorInfos)
		{
			if (!mesh)
				return false;

			const AgVertexBuffer* vb = Singleton<AgRenderResourceManager>::instance().m_vertex_buffer_pool.get(mesh->m_geometries[subId].vertex_buffer_handle);
			const AgIndexBuffer* ib = Singleton<AgRenderResourceManager>::instance().m_index_buffer_pool.get(mesh->m_geometries[subId].index_buffer_handle);
			if (!vb || !ib)
				return false;

			std::shared_ptr<T> renderNode(BX_NEW(entry::getAllocator(), T));
			for (uint8_t tex_stage = 0; tex_stage < AgShader::MAX_TEXTURE_SLOT_COUNT; tex_stage++)
			{
				AgTexture::Handle texture_handle = mesh->m_geometries[subId].texture_handle[tex_stage];
				if (AgTexture::kInvalidHandle == texture_handle)
					break;
				renderNode->setTexture(tex_stage, texture_handle);
			}
			if (!renderNode->appendGeometry(
				mesh->m_global_transform,
				mesh->m_geometries[subId].material_handle,
				mesh->m_pick_id,
				vb->m_decl,
				vb->m_vertex_buffer.GetData(), vb->m_vertex_buffer.GetSize() * sizeof(uint8_t),
				ib->m_index_buffer.GetData(), ib->m_index_buffer.GetSize() * sizeof(uint16_t)))
			{
				return false;
			}
			std::shared_ptr<AgRenderMeshEvaluator::MeshEvaluatorInfo> evaluatorInfo(new AgRenderMeshEvaluator::MeshEvaluatorInfo(renderNode, mesh->m_handle, mesh->m_geometries[subId]));
			evaluatorInfos.push_back(evaluatorInfo);
			return true;
		}

		template<typename T>
		static bool _EvaluateSubMesh(const AgMesh* mesh, EvaluateInfoArr& evaluatorInfos)
		{
			if (!mesh)
				return false;

			const int nSubMeshNum = (const int)mesh->m_geometries.size();
			if (0 == nSubMeshNum)
				return false;

			for (int i = 0; i < nSubMeshNum; i++)
			{
				_AppendRenderNode<T>(i, mesh, evaluatorInfos);
			}
			return true;
		}
	};
}
