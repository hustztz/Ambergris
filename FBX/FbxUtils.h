/****************************************************************************************

   Copyright (C) 2015 Autodesk, Inc.
   All rights reserved.

   Use of this software is subject to the terms of the Autodesk license agreement
   provided at the time of installation or download, or which otherwise accompanies
   this software in either electronic or hard copy form.

****************************************************************************************/
#ifndef _FBX_UTILS_H
#define _FBX_UTILS_H

#include <fbxsdk.h>

namespace ambergris_fbx {
	void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
	void DestroySdkObjects(FbxManager* pManager, FbxScene*& pScene);

	bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pFileFormat = -1, bool pEmbedMedia = false);
	bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
}
#endif // #ifndef _FBX_UTILITIES_H


