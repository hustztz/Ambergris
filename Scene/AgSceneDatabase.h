#pragma once

#include "AgMesh.h"
#include "Foundation/Singleton.h"
#include "Resource/AgResourceManager.h"

namespace ambergris {

	struct AgSceneDatabase : public AgResourceManager<AgObject>
	{
	public:
	private:
		AgSceneDatabase() {};
		~AgSceneDatabase() {}
		friend class Singleton<AgSceneDatabase>;
	};
}