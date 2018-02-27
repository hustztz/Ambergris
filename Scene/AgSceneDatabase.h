#pragma once

#include "AgMesh.h"
#include "Foundation/Singleton.h"
#include "Resource/AgResourcePool.h"

namespace ambergris {

	struct AgSceneDatabase : public AgResourcePool<AgObject>
	{
	public:
	private:
		AgSceneDatabase() {};
		~AgSceneDatabase() {}
		friend class Singleton<AgSceneDatabase>;
	};
}