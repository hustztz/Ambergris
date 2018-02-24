#pragma once

#include <fbxsdk.h>
#include <functional>
#include <map>

#include "Resource/AgResource.h"
#include "Scene/AgSceneImportOptions.h"

namespace ambergris {
	struct AgMesh;
}

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
		void _ImportSceneInfo(FbxScene *fbx_scene);
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

		// Vars
		FbxString mProjectPath; // current project path deducted from the fbx_filename (anything up to "/units")
		FbxString mFullUnitName;	// mUnitPath + mUnitName. Used for warning messages.

		SmtFbxMeshCBFunc m_meshCBFunc;
	};
}