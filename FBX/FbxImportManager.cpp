#include "FbxImportManager.h"
#include "FbxUtils.h"
#include "Scene\AgSceneDatabase.h"
#include "Resource\AgGeometryResourceManager.h"
#include "Resource\AgTexture.h"
#include "Resource\AgMaterial.h"

#include "BGFX/entry/entry.h"//TODO

#include <assert.h>
#include <vector>

#ifdef FBX_IMPORT_PROFILE_ENABLE
#include "profiler/code_profiler.h"
#endif

#ifdef FBX_IMPORT_PROFILE_ENABLE
#define FBX_IMPORT_PROFILE(x) CODE_PROFILE(x);
#define FBX_IMPORT_PROFILE2(x,y) CODE_PROFILE2(x,y);
#else
#define FBX_IMPORT_PROFILE(x) {}
#define FBX_IMPORT_PROFILE2(x,y) {}
#endif

using namespace ambergris;
using namespace std;

namespace ambergris_fbx {

	FbxImportManager::FbxImportManager()
		: m_pSdkManager(NULL)
		, m_pScene(NULL)
		, m_meshCBFunc(nullptr)
	{
		// Prepare the FBX SDK.
		InitializeSdkObjects(m_pSdkManager, m_pScene);
		m_originPosition.SetIdentity();
	}

	FbxImportManager::~FbxImportManager()
	{
		// Destroy all objects created by the FBX SDK.
		DestroySdkObjects(m_pSdkManager, m_pScene);
	}

	void FbxImportManager::_ImportSceneInfo(FbxScene *fbx_scene)
	{
		FbxDocumentInfo *info = fbx_scene->GetSceneInfo();
		if (!info)
			return;

		/*sdb->properties["ApplicationVendor"] = info->LastSaved_ApplicationVendor.Get().Buffer();
		sdb->properties["ApplicationName"] = info->LastSaved_ApplicationName.Get().Buffer();
		sdb->properties["ApplicationVersion"] = info->LastSaved_ApplicationVersion.Get().Buffer();

		FbxPropertyT<FbxString> active_project = info->Original.Find("ApplicationActiveProject");
		if (active_project.IsValid())
			sdb->properties["ApplicationActiveProject"] = active_project.Get().Buffer();

		FbxPropertyT<FbxString> native_file = info->Original.Find("ApplicationNativeFile");
		if (native_file.IsValid())
			sdb->properties["ApplicationNativeFile"] = native_file.Get().Buffer();*/
	}

	bool FbxImportManager::Load(const char* pFileName)
	{
		if (!m_pSdkManager || !m_pScene || !pFileName)
			return false;

		FBXSDK_printf("\n\nLoading File: %s\n\n", pFileName);

		FbxString CleanFileName = pFileName;
#ifdef FBXSDK_ENV_WIN	//Remove all quotes characters on Windows
		FbxRemoveChar(CleanFileName, '"');
#endif
		CleanFileName = FbxPathUtils::Clean(CleanFileName.Buffer());
		
		FbxString unitPath;    // current unit path deducted from the fbx_filename. Used to create the extraction folder for embedded textures
								// as well as the several other files required by stingray (*.material, *.texture, animations/*.fbx)
		FbxString unitName;    // unit name (fbx_filename without exension)
		// unit path
		unitPath = FbxPathUtils::GetFolderName(CleanFileName.Buffer());
		bool skip_dump_anim_pass = (unitPath.Find("animations") != -1);

		// project path (terminated by a / (or \ if on windows)
		mProjectPath = CleanFileName.Left(CleanFileName.Find("units"));
		if (mProjectPath.IsEmpty()) {
			// in the case the file structure does not comply to the standard (i.e missing the "units" folder)
			// we assume that the projectPath is the same as the unit path.
			FbxString temp = FbxPathUtils::Bind(unitPath, "dummy");
			mProjectPath = temp.Left(temp.Find("dummy"));
		}
		mFullUnitName = FbxPathUtils::Bind(unitPath, unitName);

		return LoadScene(m_pSdkManager, m_pScene, pFileName);
	}

	bool FbxImportManager::Save(const char* pFileName)
	{
		if (!m_pSdkManager || !m_pScene || !pFileName)
			return false;
		FBXSDK_printf("\n\nSaving File: %s\n\n", pFileName);
		return SaveScene(m_pSdkManager, m_pScene, pFileName, m_pSdkManager->GetIOPluginRegistry()->FindWriterIDByDescription("FBX ascii (*.fbx)"));
	}

	double FbxImportManager::GetUnitScale() const
	{
		if (!m_pScene)
			return 0.0;

		const FbxGlobalSettings& globals = m_pScene->GetGlobalSettings();
		FbxSystemUnit unit = globals.GetOriginalSystemUnit();
		return unit.GetScaleFactor() * unit.GetMultiplier();
	}

	int	FbxImportManager::GetUpAxis()
	{
		if (!m_pScene)
			return FbxAxisSystem::eZAxis;

		FbxAxisSystem axis = m_pScene->GetGlobalSettings().GetAxisSystem();
		int pSign = 1;
		return axis.GetUpVector(pSign);
	}

	bool is_visible(FbxNode &fbx_node)
	{
		if (!fbx_node.Show.Get() || fbx_node.Visibility.Get() == 0)
			return false;

		FbxNode *parent = fbx_node.GetParent();
		if (parent && !is_visible(*parent))
			return false;

		return true;
	}

	bool mesh_is_empty(const FbxMesh *fbx_mesh)
	{
		return fbx_mesh == nullptr || fbx_mesh->GetControlPointsCount() == 0 || fbx_mesh->GetPolygonCount() == 0;
	}

	void FbxImportManager::_TraverseNodeRecursive(FbxNode* node)
	{
		const char* nodeName = node->GetName();
		printf("node name: %s\n", nodeName);
		// Get da transforms
		/*FbxDouble3 translation = node->LclTranslation.Get();
		FbxDouble3 rotation = node->LclRotation.Get();
		FbxDouble3 scaling = node->LclScaling.Get();*/

		// Determine # of children the node has
		int numChildren = node->GetChildCount();
		FbxNode* childNode = 0;
		for (int i = 0; i < numChildren; i++)
		{
			childNode = node->GetChild(i);
			if (childNode->GetNodeAttribute())
			{
				FbxNodeAttribute::EType lAttributeType = childNode->GetNodeAttribute()->GetAttributeType();
				switch (lAttributeType)
				{
				case FbxNodeAttribute::eMesh:
					_ParseMesh(childNode);
					break;
				default:
					break;
				}
			}
			_TraverseNodeRecursive(childNode);
		}
	}

	bool FbxImportManager::ParseScene()
	{
		if (!m_pScene)
			return false;

		FBX_IMPORT_PROFILE("[FBX] ImportFbxSceneToSceneDatabase");

		_ImportSceneInfo(m_pScene);

		/*FbxGlobalSettings &global_settings = m_pScene->GetGlobalSettings();
		FbxSystemUnit file_original = global_settings.GetOriginalSystemUnit();
		file_original.ConvertScene(m_pScene);*/

		//// Convert Axis System to what is used in this example, if needed
		//FbxAxisSystem SceneAxisSystem = m_pScene->GetGlobalSettings().GetAxisSystem();
		//FbxAxisSystem OurAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eRightHanded);
		//if (SceneAxisSystem != OurAxisSystem)
		//{
		//	OurAxisSystem.ConvertScene(m_pScene);
		//}

		//// Convert Unit System to what is used in this example, if needed
		//FbxSystemUnit SceneSystemUnit = m_pScene->GetGlobalSettings().GetSystemUnit();
		//if (SceneSystemUnit.GetScaleFactor() != 1.0)
		//{
		//	//The unit in this example is centimeter.
		//	FbxSystemUnit::cm.ConvertScene(m_pScene);
		//}

		// Triangulate all meshes that are not triangulated
		{
			FBX_IMPORT_PROFILE("[FBX] Triangulate");
			FbxGeometryConverter GeomConverter(m_pSdkManager);
			GeomConverter.RemoveBadPolygonsFromMeshes(m_pScene);
			GeomConverter.Triangulate(m_pScene, true);
		}

		FbxNode* lRootNode = m_pScene->GetRootNode();
		m_originPosition = lRootNode->EvaluateGlobalTransform();
		m_instance_map.clear();
		_TraverseNodeRecursive(lRootNode);
		m_instance_map.clear();

		return true;
	}

	uint32_t rgbaToAbgr(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a)
	{
		return (uint32_t(_r) << 0)
			| (uint32_t(_g) << 8)
			| (uint32_t(_b) << 16)
			| (uint32_t(_a) << 24)
			;
	}

	template<class T> int GetLayerElementIndex(const FbxLayerElementTemplate<T> *pLayerElement, int pControlPointIndex, int pPolyVertexIndex, int pPolyIndex)
	{
		int ElementIndex = 0;
		switch (pLayerElement->GetMappingMode()) {
		case FbxLayerElement::eByControlPoint:	ElementIndex = pControlPointIndex; break;
		case FbxLayerElement::eByPolygonVertex:	ElementIndex = pPolyVertexIndex; break;
		case FbxLayerElement::eByPolygon:		ElementIndex = pPolyIndex; break;
		default:								FBX_ASSERT_NOW("Unsupported mapping mode"); break;
		}
		if (pLayerElement->GetReferenceMode() != FbxLayerElement::eDirect) {
			ElementIndex = pLayerElement->GetIndexArray().GetAt(ElementIndex);
		}
		return ElementIndex;
	}

	void FbxImportManager::_ExtractMesh(ambergris::AgMesh& renderNode, FbxMesh *pMesh)
	{
		if (!pMesh)
			return;

		/*int numVertices = pMesh->GetControlPointsCount();
		for (int i = 0; i < numVertices; i++)
		{
		FbxVector4 coord = pMesh->GetControlPointAt(i);
		SmtVertex vtx;
		vtx.vt[0] = (float)coord.mData[i*SmtMesh::VERTEX_STRIDE];
		vtx.vt[1] = (float)coord.mData[i*SmtMesh::VERTEX_STRIDE + 1];
		vtx.vt[2] = (float)coord.mData[i*SmtMesh::VERTEX_STRIDE + 2];
		vtx.vt[3] = 1.0f;
		renderMesh.m_vertex_data.push_back(vtx);
		}*/

		const int n_polygonCount = pMesh->GetPolygonCount();
		if (n_polygonCount <= 0)
			return;

		struct SubMesh
		{
			SubMesh() : m_material_index(-1) {};
			int		m_material_index;
			vector<int>	m_primitives;
		};
		vector<SubMesh>	subMeshes;

		FbxLayerElementArrayTemplate<int>* lMaterialIndice = NULL;
		FbxLayerElement::EMappingMode lMaterialMappingMode = FbxGeometryElement::eNone;
		const int n_materials = pMesh->GetNode()->GetMaterialCount();
		// Material ids
		if (n_materials > 0)
		{
			FbxGeometryElementMaterial *lMaterialElement = pMesh->GetElementMaterial();
			if (lMaterialElement)
			{
				lMaterialMappingMode = lMaterialElement->GetMappingMode();
				if (lMaterialMappingMode == FbxLayerElement::eAllSame) {
					lMaterialIndice = &lMaterialElement->GetIndexArray();
					int material_id = lMaterialIndice->GetAt(0);
					if (material_id < 0 || material_id >= (int)n_materials) {
						material_id = 0;
					}
					SubMesh subMesh;
					subMesh.m_material_index = material_id;
					subMesh.m_primitives.clear();
					for (int poly_index = 0; poly_index < n_polygonCount; poly_index++) {
						subMesh.m_primitives.push_back(poly_index);
					}
					subMeshes.push_back(subMesh);
				}
				else if (lMaterialMappingMode == FbxLayerElement::eByPolygon) {
					lMaterialIndice = &lMaterialElement->GetIndexArray();
					int n_material_ids = lMaterialIndice->GetCount();
					subMeshes.resize(n_materials);
					for (int poly_index = 0; poly_index < n_polygonCount; poly_index++) {
						int material_id = 0;
						if (poly_index < n_material_ids)
						{
							int material_id = lMaterialIndice->GetAt(poly_index);
							if (material_id < 0 || material_id >= (int)n_materials) {
								material_id = 0;
							}
						}
						subMeshes[material_id].m_material_index = material_id;
						subMeshes[material_id].m_primitives.push_back(poly_index);
					}
				}
			}
		}

		const int TRIANGLE_VERTEX_COUNT = 3;
		// Three floats for every position.
		const int VERTEX_STRIDE = 3;
		// Three floats for every normal.
		const int NORMAL_STRIDE = 3;
		// Four floats for every tangent.
		const int TANGENT_STRIDE = 4;
		// Four floats for every color.
		const int COLOR_STRIDE = 4;
		// Two floats for every UV.
		const int UV_STRIDE = 2;

		struct AgFbxMesh
		{
			AgFbxMesh() : m_bAllByControlPoint(false) {
			};

			bool					m_bAllByControlPoint;	// Save data in VBO by control point or by polygon vertex.
			bgfx::VertexDecl		m_decl;
			TBuffer<uint8_t>		m_vertex_buffer;
			TBuffer<uint16_t>		m_index_buffer;

		};

		AgFbxMesh fbxMesh;
		// Congregate all the data of a mesh to be cached in VBOs.
		// If normal or UV is by polygon vertex, record all vertex attributes by polygon vertex.
		bool hasNormal = pMesh->GetElementNormalCount() > 0;
		bool hasTangent = pMesh->GetElementTangentCount() > 0;
		bool hasBinormal = pMesh->GetElementBinormalCount() > 0;
		bool hasUV = pMesh->GetElementUVCount() > 0;
		FbxGeometryElement::EMappingMode lNormalMappingMode = FbxGeometryElement::eNone;
		FbxGeometryElement::EMappingMode lUVMappingMode = FbxGeometryElement::eNone;
		if (hasNormal)
		{
			lNormalMappingMode = pMesh->GetElementNormal(0)->GetMappingMode();
			if (lNormalMappingMode == FbxGeometryElement::eNone)
			{
				hasNormal = false;
			}
			if (hasNormal && lNormalMappingMode != FbxGeometryElement::eByControlPoint)
			{
				fbxMesh.m_bAllByControlPoint = false;
			}
		}
		const char * lUVName = NULL;
		if (hasUV)
		{
			lUVMappingMode = pMesh->GetElementUV(0)->GetMappingMode();
			if (lUVMappingMode == FbxGeometryElement::eNone)
			{
				hasUV = false;
			}
			if (hasUV && lUVMappingMode != FbxGeometryElement::eByControlPoint)
			{
				fbxMesh.m_bAllByControlPoint = false;
			}

			FbxStringList lUVNames;
			pMesh->GetUVSetNames(lUVNames);
			if (hasUV && lUVNames.GetCount())
			{
				lUVName = lUVNames[0];
			}
			else
			{
				hasUV = false;
			}
		}

		// Allocate streams on target Geometry object
		fbxMesh.m_decl.begin();
		fbxMesh.m_decl.add(bgfx::Attrib::Position, VERTEX_STRIDE, bgfx::AttribType::Float);

		if (hasNormal)
		{
			if (m_import_options.m_pack_normal)
			{
				fbxMesh.m_decl.add(bgfx::Attrib::Normal, NORMAL_STRIDE, bgfx::AttribType::Uint8, true, true);
				if (hasTangent)
				{
					fbxMesh.m_decl.add(bgfx::Attrib::Tangent, TANGENT_STRIDE, bgfx::AttribType::Uint8, true, true);
				}
				if (hasBinormal)
				{
					fbxMesh.m_decl.add(bgfx::Attrib::Bitangent, TANGENT_STRIDE, bgfx::AttribType::Uint8, true, true);
				}
			}
			else
			{
				fbxMesh.m_decl.add(bgfx::Attrib::Normal, NORMAL_STRIDE, bgfx::AttribType::Float);
				if (hasTangent)
				{
					fbxMesh.m_decl.add(bgfx::Attrib::Tangent, TANGENT_STRIDE, bgfx::AttribType::Float);
				}
				if (hasBinormal)
				{
					fbxMesh.m_decl.add(bgfx::Attrib::Bitangent, TANGENT_STRIDE, bgfx::AttribType::Float);
				}
			}
		}
		if (hasUV)
		{
			int n_uv_channels = pMesh->GetUVLayerCount();
			if (n_uv_channels > m_import_options.m_max_uv_channels) {
				n_uv_channels = m_import_options.m_max_uv_channels;
			}
			for (int i = 0; i < n_uv_channels; i++) {
				if (m_import_options.m_pack_uv)
				{
					fbxMesh.m_decl.add(bgfx::Attrib::Enum((int)bgfx::Attrib::TexCoord0 + i), UV_STRIDE, bgfx::AttribType::Half);
				}
				else
				{
					fbxMesh.m_decl.add(bgfx::Attrib::Enum((int)bgfx::Attrib::TexCoord0 + i), UV_STRIDE, bgfx::AttribType::Float);
				}
			}
		}

		const int max_color_channels = 1;
		int n_color_channels = pMesh->GetElementVertexColorCount();
		if (n_color_channels > max_color_channels) {
			n_color_channels = max_color_channels;
		}
		//for (int i = 0; i < n_color_channels; i++)
		if (n_color_channels > 1)
		{
			fbxMesh.m_decl.add(bgfx::Attrib::Color0, COLOR_STRIDE, bgfx::AttribType::Uint8, true);
		}
		fbxMesh.m_decl.end();
		const uint16_t stride = fbxMesh.m_decl.getStride();
		// TODO
		fbxMesh.m_bAllByControlPoint = false;

		// Allocate the array memory, by control point or by polygon vertex.
		int nPolygonVertexCount = pMesh->GetControlPointsCount();
		if (!fbxMesh.m_bAllByControlPoint)
		{
			nPolygonVertexCount = n_polygonCount * TRIANGLE_VERTEX_COUNT;
		}
		fbxMesh.m_vertex_buffer.Resize(nPolygonVertexCount * stride * sizeof(uint8_t));
		uint8_t* vertexData = const_cast<uint8_t*>(fbxMesh.m_vertex_buffer.GetData());
		fbxMesh.m_index_buffer.Resize(nPolygonVertexCount * sizeof(uint16_t));
		uint16_t* indexData = const_cast<uint16_t*>(fbxMesh.m_index_buffer.GetData());

		// Populate the array with vertex attribute, if by control point.
		const FbxVector4 * lControlPoints = pMesh->GetControlPoints();
		FbxVector4 lCurrentVertex;
		FbxVector4 lCurrentNormal;
		FbxVector2 lCurrentUV;
		FbxColor lCurrentColor;
		if (fbxMesh.m_bAllByControlPoint)
		{
			const FbxGeometryElementNormal * lNormalElement = NULL;
			const FbxGeometryElementUV * lUVElement = NULL;
			const FbxGeometryElementVertexColor * lColorElement = NULL;
			if (fbxMesh.m_decl.has(bgfx::Attrib::Normal))
			{
				lNormalElement = pMesh->GetElementNormal(0);
			}
			if (fbxMesh.m_decl.has(bgfx::Attrib::TexCoord0))
			{
				lUVElement = pMesh->GetElementUV(0);
			}
			if (fbxMesh.m_decl.has(bgfx::Attrib::TexCoord0))
			{
				lColorElement = pMesh->GetElementVertexColor(0);
			}
			for (int nIndex = 0; nIndex < nPolygonVertexCount; ++nIndex)
			{
				// Save the vertex position.
				lCurrentVertex = lControlPoints[nIndex];
				uint16_t positionOffset = fbxMesh.m_decl.getOffset(bgfx::Attrib::Position);
				float* pos = reinterpret_cast<float*>(vertexData + stride * nIndex + positionOffset);
				pos[0] = static_cast<float>(lCurrentVertex[0]);
				pos[1] = static_cast<float>(lCurrentVertex[1]);
				pos[2] = static_cast<float>(lCurrentVertex[2]);

				// Save the normal.
				if (fbxMesh.m_decl.has(bgfx::Attrib::Normal) && lNormalElement)
				{
					int lNormalIndex = nIndex;
					if (lNormalElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						lNormalIndex = lNormalElement->GetIndexArray().GetAt(nIndex);
					}
					lCurrentNormal = lNormalElement->GetDirectArray().GetAt(lNormalIndex);

					uint8_t num;
					bgfx::AttribType::Enum type;
					bool normalized;
					bool asInt;
					fbxMesh.m_decl.decode(bgfx::Attrib::Normal, num, type, normalized, asInt);
					if (bgfx::AttribType::Float == type)
					{
						uint16_t normalOffset = fbxMesh.m_decl.getOffset(bgfx::Attrib::Normal);
						float* norm = reinterpret_cast<float*>(vertexData + stride * nIndex + normalOffset);
						norm[0] = static_cast<float>(lCurrentNormal[0]);
						norm[1] = static_cast<float>(lCurrentNormal[1]);
						norm[2] = static_cast<float>(lCurrentNormal[2]);
					}
					else
					{
						float norm[3];
						norm[0] = static_cast<float>(lCurrentNormal[0]);
						norm[1] = static_cast<float>(lCurrentNormal[1]);
						norm[2] = static_cast<float>(lCurrentNormal[2]);
						bgfx::vertexPack(norm, true, bgfx::Attrib::Normal, fbxMesh.m_decl, vertexData, nIndex);
					}
				}

				// Save the UV.
				if (fbxMesh.m_decl.has(bgfx::Attrib::TexCoord0) && lUVElement)
				{
					int lUVIndex = nIndex;
					if (lUVElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						lUVIndex = lUVElement->GetIndexArray().GetAt(nIndex);
					}
					lCurrentUV = lUVElement->GetDirectArray().GetAt(lUVIndex);

					uint8_t num;
					bgfx::AttribType::Enum type;
					bool normalized;
					bool asInt;
					fbxMesh.m_decl.decode(bgfx::Attrib::TexCoord0, num, type, normalized, asInt);
					if (bgfx::AttribType::Float == type)
					{
						uint16_t uvOffset = fbxMesh.m_decl.getOffset(bgfx::Attrib::TexCoord0);
						float* uv = reinterpret_cast<float*>(vertexData + stride * nIndex + uvOffset);
						uv[0] = static_cast<float>(lCurrentUV[0]);
						uv[1] = static_cast<float>(lCurrentUV[1]);
					}
					else
					{
						float uv[2];
						uv[0] = static_cast<float>(lCurrentUV[0]);
						uv[1] = static_cast<float>(lCurrentUV[1]);
						bgfx::vertexPack(uv, true, bgfx::Attrib::TexCoord0, fbxMesh.m_decl, vertexData, nIndex);
					}
				}
				// Save the color.
				if (fbxMesh.m_decl.has(bgfx::Attrib::Color0) && lColorElement)
				{
					int lColorIndex = nIndex;
					if (lColorElement->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					{
						lColorIndex = lColorElement->GetIndexArray().GetAt(nIndex);
					}
					lCurrentColor = lColorElement->GetDirectArray().GetAt(lColorIndex);

					uint16_t colorOffset = fbxMesh.m_decl.getOffset(bgfx::Attrib::Color0);
					uint32_t* col = reinterpret_cast<uint32_t*>(vertexData + stride * nIndex + colorOffset);
					*col = rgbaToAbgr((int)(lCurrentColor[0] * 255) % 255, (int)(lCurrentColor[1] * 255) % 255, (int)(lCurrentColor[2] * 255) % 255, 0xff);
				}
			}

		}

		for (int nPolygonIndex = 0; nPolygonIndex < n_polygonCount; ++nPolygonIndex)
		{
			for (int nVerticeIndex = 0; nVerticeIndex < TRIANGLE_VERTEX_COUNT; ++nVerticeIndex)
			{
				const int nControlPointIndex = pMesh->GetPolygonVertex(nPolygonIndex, nVerticeIndex);
				const int nIndex = nPolygonIndex*TRIANGLE_VERTEX_COUNT + nVerticeIndex;
				if (fbxMesh.m_bAllByControlPoint)
				{
					indexData[nIndex] = static_cast<unsigned int>(nControlPointIndex);
				}
				// Populate the array with vertex attribute, if by polygon vertex.
				else
				{
					indexData[nIndex] = static_cast<unsigned int>(nIndex);

					lCurrentVertex = lControlPoints[nControlPointIndex];
					uint16_t positionOffset = fbxMesh.m_decl.getOffset(bgfx::Attrib::Position);
					float* pos = reinterpret_cast<float*>(vertexData + stride * nIndex + positionOffset);
					pos[0] = static_cast<float>(lCurrentVertex[0]);
					pos[1] = static_cast<float>(lCurrentVertex[1]);
					pos[2] = static_cast<float>(lCurrentVertex[2]);

					// Save the normal.
					if (fbxMesh.m_decl.has(bgfx::Attrib::Normal))
					{
						pMesh->GetPolygonVertexNormal(nPolygonIndex, nVerticeIndex, lCurrentNormal);

						uint8_t num;
						bgfx::AttribType::Enum type;
						bool normalized;
						bool asInt;
						fbxMesh.m_decl.decode(bgfx::Attrib::Normal, num, type, normalized, asInt);
						if (bgfx::AttribType::Float == type)
						{
							uint16_t normalOffset = fbxMesh.m_decl.getOffset(bgfx::Attrib::Normal);
							float* norm = reinterpret_cast<float*>(vertexData + stride * nIndex + normalOffset);
							norm[0] = static_cast<float>(lCurrentNormal[0]);
							norm[1] = static_cast<float>(lCurrentNormal[1]);
							norm[2] = static_cast<float>(lCurrentNormal[2]);
						}
						else
						{
							float norm[3];
							norm[0] = static_cast<float>(lCurrentNormal[0]);
							norm[1] = static_cast<float>(lCurrentNormal[1]);
							norm[2] = static_cast<float>(lCurrentNormal[2]);
							bgfx::vertexPack(norm, true, bgfx::Attrib::Normal, fbxMesh.m_decl, vertexData, nIndex);
						}
					}
					// Save the UV.
					if (fbxMesh.m_decl.has(bgfx::Attrib::TexCoord0) && lUVName)
					{
						bool lUnmappedUV;
						pMesh->GetPolygonVertexUV(nPolygonIndex, nVerticeIndex, lUVName, lCurrentUV, lUnmappedUV);

						uint8_t num;
						bgfx::AttribType::Enum type;
						bool normalized;
						bool asInt;
						fbxMesh.m_decl.decode(bgfx::Attrib::TexCoord0, num, type, normalized, asInt);
						if (bgfx::AttribType::Float == type)
						{
							uint16_t uvOffset = fbxMesh.m_decl.getOffset(bgfx::Attrib::TexCoord0);
							float* uv = reinterpret_cast<float*>(vertexData + stride * nIndex + uvOffset);
							uv[0] = static_cast<float>(lCurrentUV[0]);
							uv[1] = static_cast<float>(lCurrentUV[1]);
						}
						else
						{
							float uv[2];
							uv[0] = static_cast<float>(lCurrentUV[0]);
							uv[1] = static_cast<float>(lCurrentUV[1]);
							bgfx::vertexPack(uv, true, bgfx::Attrib::TexCoord0, fbxMesh.m_decl, vertexData, nIndex);
						}
					}
					// Save the color.
					if (fbxMesh.m_decl.has(bgfx::Attrib::Color0))
					{
						const FbxLayerElementVertexColor *Colors = pMesh->GetElementVertexColor(0);
						if (Colors)
						{
							const FbxColor &Color = Colors->GetDirectArray()[GetLayerElementIndex(Colors, nControlPointIndex, nIndex, nPolygonIndex)];
							uint16_t colorOffset = fbxMesh.m_decl.getOffset(bgfx::Attrib::Color0);
							uint32_t* col = reinterpret_cast<uint32_t*>(vertexData + stride * nIndex + colorOffset);
							*col = rgbaToAbgr((int)(Color[0] * 255) % 255, (int)(Color[1] * 255) % 255, (int)(Color[2] * 255) % 255, 0xff);
						}
					}
				}
			}
		}

		// To ag database
		if (subMeshes.empty())
		{
			AgVertexBuffer* vb = Singleton<AgGeometryResourceManager>::instance().m_vertex_buffer_pool.allocate<AgVertexBuffer>(entry::getAllocator());
			AgIndexBuffer* ib = Singleton<AgGeometryResourceManager>::instance().m_index_buffer_pool.allocate<AgIndexBuffer>(entry::getAllocator());
			if (vb && ib)
			{
				ib->m_bAllByControlPoint = fbxMesh.m_bAllByControlPoint;
				ib->m_index_buffer = fbxMesh.m_index_buffer;
				vb->m_decl = fbxMesh.m_decl;
				vb->m_vertex_buffer = fbxMesh.m_vertex_buffer;
				AgMesh::Geometry scene_geom;
				scene_geom.material_handle = AgMaterial::E_LAMBERT;
				scene_geom.vertex_buffer_handle = vb->m_handle;
				scene_geom.index_buffer_handle = ib->m_handle;
				renderNode.m_geometries.push_back(scene_geom);
			}
		}
		else
		{
			const unsigned int index_buffer_size = fbxMesh.m_index_buffer.GetSize();
			for (int i = 0; i < subMeshes.size(); ++i)
			{
				const SubMesh& sub_mesh = subMeshes[i];
				const int nSubMeshPolyCount = (const int)sub_mesh.m_primitives.size();
				if(0 == nSubMeshPolyCount)
					continue;

				AgVertexBuffer* vb = Singleton<AgGeometryResourceManager>::instance().m_vertex_buffer_pool.allocate<AgVertexBuffer>(entry::getAllocator());
				AgIndexBuffer* ib = Singleton<AgGeometryResourceManager>::instance().m_index_buffer_pool.allocate<AgIndexBuffer>(entry::getAllocator());
				if (!vb || !ib)
					break;

				vb->m_decl = fbxMesh.m_decl;
				vb->m_vertex_buffer = fbxMesh.m_vertex_buffer;

				ib->m_bAllByControlPoint = fbxMesh.m_bAllByControlPoint;
				ib->m_index_buffer.Resize(nSubMeshPolyCount * TRIANGLE_VERTEX_COUNT  * sizeof(uint16_t));
				uint16_t* indexData = const_cast<uint16_t*>(fbxMesh.m_index_buffer.GetData());
				for (int prim_id = 0; prim_id < nSubMeshPolyCount; ++ prim_id)
				{
					const int poly_idx = sub_mesh.m_primitives[prim_id];
					if(poly_idx < 0 || poly_idx >= n_polygonCount)
						continue;
					for (int nVerticeIndex = 0; nVerticeIndex < TRIANGLE_VERTEX_COUNT; ++nVerticeIndex)
					{
						const int nDstIndex = prim_id*TRIANGLE_VERTEX_COUNT + nVerticeIndex;
						if (sizeof(uint16_t) * nDstIndex >= index_buffer_size)
							continue;
						if (fbxMesh.m_bAllByControlPoint)
						{
							const int nControlPointIndex = pMesh->GetPolygonVertex(poly_idx, nVerticeIndex);
							uint16_t* dst_index_data = (uint16_t*)ib->m_index_buffer.GetData() + nDstIndex;
							*dst_index_data = nControlPointIndex;
						}
						else
						{
							const int nSrcIndex = poly_idx*TRIANGLE_VERTEX_COUNT + nVerticeIndex;
							if (sizeof(uint16_t) * nSrcIndex >= index_buffer_size)
								continue;
							uint16_t* src_index_data = indexData + nSrcIndex;
							uint16_t* dst_index_data = (uint16_t*)ib->m_index_buffer.GetData() + nDstIndex;
							*dst_index_data = *src_index_data;
						}
					}
				}

				AgMesh::Geometry scene_geom;
				scene_geom.vertex_buffer_handle = vb->m_handle;
				scene_geom.index_buffer_handle = ib->m_handle;
				switch (sub_mesh.m_material_index)
				{
				case 1:
					scene_geom.material_handle = AgMaterial::E_LAMBERT;
					break;
				default:
					scene_geom.material_handle = AgMaterial::E_LAMBERT;
					break;
				}
				renderNode.m_geometries.push_back(scene_geom);
			}
		}
	}

	void FbxImportManager::_ParseMesh(FbxNode* node)
	{
		if (!node)
			return;
		FbxMesh *pMesh = node->GetMesh();
		if (!pMesh)
			return;
		if (!m_meshCBFunc)
			return;

		// Output results
		AgResource::Handle parent_handle = AgResource::kInvalidHandle;
		AgSceneDatabase& agScene = Singleton<AgSceneDatabase>::instance();
		for (int i = 0; i < agScene.getSize(); ++i)
		{
			const AgMesh* tmpNode = dynamic_cast<const AgMesh*>(agScene.get(i));
			if(!tmpNode)
				continue;
			if (std::strcmp(node->GetParent()->GetName(), tmpNode->m_name.c_str()))
			{
				parent_handle = tmpNode->m_handle;
				break;
			}
		}

		AgMesh* renderNode = dynamic_cast<AgMesh*>(agScene.allocate<AgMesh>(entry::getAllocator()));
		if (!renderNode)
			return;
		renderNode->m_name = stl::string(node->GetName());
		renderNode->m_parent_handle = parent_handle;

		// Transform
		FbxAMatrix lGlobalPosition = pMesh->GetNode()->EvaluateGlobalTransform();
		// TODO: need precompute
		if (m_originPosition.IsIdentity())
		{
			m_originPosition = lGlobalPosition;
		}
		for (int m = 0; m < 4; m++)
		{
			for (int n = 0; n < 4; n++)
			{
				renderNode->m_global_transform[m*4+n] = (float)lGlobalPosition.Get(m, n);
			}
		}
		renderNode->m_global_transform[12] = renderNode->m_global_transform[12] - (float)m_originPosition.Get(3, 0);
		renderNode->m_global_transform[13] = renderNode->m_global_transform[13] - (float)m_originPosition.Get(3, 1);
		renderNode->m_global_transform[14] = renderNode->m_global_transform[14] - (float)m_originPosition.Get(3, 2);
		FbxAMatrix lLocalPosition = pMesh->GetNode()->EvaluateLocalTransform();
		for (int m = 0; m < 4; m++)
		{
			for (int n = 0; n < 4; n++)
			{
				renderNode->m_local_transform[m * 4 + n] = (float)lLocalPosition.Get(m, n);
			}
		}
		renderNode->m_local_transform[12] = renderNode->m_local_transform[12] - (float)m_originPosition.Get(3, 0);
		renderNode->m_local_transform[13] = renderNode->m_local_transform[13] - (float)m_originPosition.Get(3, 1);
		renderNode->m_local_transform[14] = renderNode->m_local_transform[14] - (float)m_originPosition.Get(3, 2);

		auto it_instance = m_instance_map.find(pMesh);
		if (it_instance != m_instance_map.end())
		{
			AgMesh* first_node = dynamic_cast<AgMesh*>(agScene.get(it_instance->second));
			assert(first_node);
			if (first_node)
			{
				first_node->m_inst_handle = 1;
				renderNode->m_inst_handle = 1;
				renderNode->m_geometries = first_node->m_geometries;
				m_meshCBFunc(renderNode);
			}
		}
		else
		{
			_ExtractMesh(*renderNode, pMesh);
			renderNode->evaluateBoundingBox();
			m_meshCBFunc(renderNode);
			m_instance_map.insert(InstanceMap::value_type(pMesh, renderNode->m_handle));
		}
		
		return;
	}

	void FbxImportManager::_ParseTexture(FbxScene *pScene)
	{
		if (pScene)
			return;

		const int lTextureCount = pScene->GetTextureCount();
		//for (int lTextureIndex = 0; lTextureIndex < lTextureCount; ++lTextureIndex)
		//{
		//	FbxTexture * lTexture = pScene->GetTexture(lTextureIndex);
		//	FbxFileTexture * lFileTexture = FbxCast<FbxFileTexture>(lTexture);
		//	if (lFileTexture && !lFileTexture->GetUserDataPtr())
		//	{
		//		// Try to load the texture from absolute path
		//		const FbxString lFileName = lFileTexture->GetFileName();
		//		//renderNode.m_mesh_handle = Singleton<ambergris_bgfx::BgfxTextureManager>::instance().Append(renderMesh);
		//	}
		//}
	}
}