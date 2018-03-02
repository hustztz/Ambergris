#pragma once

#include <fbxsdk.h>
#include <functional>
#include <map>

#include "Scene/AgMesh.h"
#include "Scene/AgSceneImportOptions.h"

namespace ambergris_fbx {

	class FbxImportManager
	{
	public:
		typedef std::function<void(ambergris::AgMesh*)>	SmtFbxMeshCBFunc;
	public:
		FbxImportManager();
		~FbxImportManager();

		bool Load(const char* pFileName);
		bool Save(const char* pFileName);

		void RegisterMeshCBFunc(SmtFbxMeshCBFunc func) { m_meshCBFunc = func; }

		bool ParseScene();

		double GetUnitScale() const;
		int	GetUpAxis();
	protected:
		struct Material
		{
			struct ColorChannel
			{
				ColorChannel()
				{
					mColor[0] = 0.0f;
					mColor[1] = 0.0f;
					mColor[2] = 0.0f;
					mColor[3] = 1.0f;
				}

				std::string mTexturePath;
				float mColor[4];
			};
			ColorChannel mEmissive;
			ColorChannel mAmbient;
			ColorChannel mDiffuse;
			ColorChannel mSpecular;
			float mShinness;
		};
		void _ImportSceneInfo(FbxScene *fbx_scene);
		FbxDouble3 _GetMaterialProperty(ambergris::AgMesh::Geometry* output,
			const FbxSurfaceMaterial * pMaterial,
			const char * pPropertyName,
			const char * pFactorPropertyName);
		void _ParseMaterial(ambergris::AgMesh::Geometry* output, const FbxSurfaceMaterial * pMaterial);
		void _ParseTexture(FbxScene *fbx_scene);
		void _ExtractMesh(ambergris::AgMesh& renderNode, FbxMesh *pMesh);
		void _ParseMesh(FbxNode* node);
		void _TraverseNodeRecursive(FbxNode* node);
	private:
		FbxManager* m_pSdkManager;
		FbxScene* m_pScene;

		ambergris::AgSceneImportOptions m_import_options;
		typedef std::map<FbxMesh*, ambergris::AgResource::Handle> InstanceMap;
		InstanceMap m_instance_map;
		FbxAMatrix m_originPosition;

		// Vars
		FbxString mProjectPath; // current project path deducted from the fbx_filename (anything up to "/units")
		FbxString mFullUnitName;	// mUnitPath + mUnitName. Used for warning messages.

		SmtFbxMeshCBFunc m_meshCBFunc;
	};
}