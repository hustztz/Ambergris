#pragma once

#include <fbxsdk.h>
#include <functional>
#include <map>

#include "Scene/SceneImportOptions.h"

namespace ambergris {
	struct BgfxNodeHeirarchy;
}

namespace ambergris_fbx {

	class FbxImportManager
	{
	public:
		typedef std::function<void(ambergris::BgfxNodeHeirarchy*)>	SmtFbxMeshCBFunc;
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
		void _ExtractMesh(ambergris::BgfxNodeHeirarchy& renderNode, FbxMesh *pMesh);
		void _ParseMesh(FbxNode* node);
		void _TraverseNodeRecursive(FbxNode* node);
	private:
		FbxManager* m_pSdkManager;
		FbxScene* m_pScene;

		ambergris::SceneImportOptions m_import_options;
		typedef std::map<FbxMesh*, int> InstanceMap;
		InstanceMap m_instance_map;

		// Vars
		FbxString mProjectPath; // current project path deducted from the fbx_filename (anything up to "/units")
		FbxString mFullUnitName;	// mUnitPath + mUnitName. Used for warning messages.

		SmtFbxMeshCBFunc m_meshCBFunc;
	};
}