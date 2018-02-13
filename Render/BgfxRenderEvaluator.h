#pragma once
#include "BgfxRenderSceneBridge.h"
#include "BgfxRenderNode.h"
#include "Scene\SceneGeometryResource.h"
#include "Scene\BgfxNodeHeirarchy.h"

namespace ambergris_bgfx {

	template<typename T>
	class BgfxRenderEvaluator : public BgfxRenderBaseEvaluator
	{
	public:
		BgfxRenderEvaluator(EvaluateNodeArr& nodes) : BgfxRenderBaseEvaluator(nodes)	{}
		virtual ~BgfxRenderEvaluator() {}

		virtual bool Evaluate(const ambergris::BgfxNodeHeirarchy& geomNode) override
		{
			const ambergris::BgfxMesh* pMesh = Singleton<SceneGeometryResource>::instance().GetMesh(geomNode.m_mesh_handle);
			if (!pMesh)
				return false;

			const int nSubMeshNum = (const int)geomNode.m_subMesh.size();
			if (1 == nSubMeshNum)
			{
				BgfxMaterialManager::MaterialIndex mat_id;
				switch (geomNode.m_subMesh[0].m_material_index)
				{
				case 0:
					mat_id = BgfxMaterialManager::E_LAMBERT;
				default:
					break;
				}
				T* renderNode = new T();
				renderNode->SetMaterial(mat_id);
				if (!renderNode->AppendGeometry(
					geomNode.m_global_transform,
					pMesh->m_decl,
					pMesh->m_vertex_buffer.GetData(), pMesh->m_vertex_buffer.GetSize() * sizeof(uint8_t),
					pMesh->m_index_buffer.GetData(), pMesh->m_index_buffer.GetSize() * sizeof(uint16_t)))
				{
					return false;
				}
				m_evaluate_nodes.push_back(EvaluateNode(geomNode.m_mesh_handle, renderNode));
			}
			else if (nSubMeshNum > 1)
			{
				for (int i = 0; i < nSubMeshNum; ++i)
				{
					BgfxMaterialManager::MaterialIndex mat_id;
					switch (geomNode.m_subMesh[i].m_material_index)
					{
					case 0:
						mat_id = BgfxMaterialManager::E_LAMBERT;
					default:
						break;
					}
					// TODO: index
					T* renderNode = new T();
					renderNode->SetMaterial(mat_id);
					if (!renderNode->AppendGeometry(
						geomNode.m_global_transform,
						pMesh->m_decl,
						pMesh->m_vertex_buffer.GetData(), pMesh->m_vertex_buffer.GetSize() * sizeof(uint8_t),
						pMesh->m_index_buffer.GetData(), pMesh->m_index_buffer.GetSize() * sizeof(uint16_t)))
					{
						return false;
					}
					m_evaluate_nodes.push_back(EvaluateNode(geomNode.m_mesh_handle, renderNode));
				}
			}
			else
			{
				T* renderNode = new T();
				renderNode->SetMaterial(BgfxMaterialManager::E_LAMBERT);
				if (!renderNode->AppendGeometry(
					geomNode.m_global_transform,
					pMesh->m_decl,
					pMesh->m_vertex_buffer.GetData(), pMesh->m_vertex_buffer.GetSize() * sizeof(uint8_t),
					pMesh->m_index_buffer.GetData(), pMesh->m_index_buffer.GetSize() * sizeof(uint16_t)))
				{
					return false;
				}
				m_evaluate_nodes.push_back(EvaluateNode(geomNode.m_mesh_handle, renderNode));
			}
			return true;
		}
	};
}
