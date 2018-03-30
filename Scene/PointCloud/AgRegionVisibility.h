#pragma once

#include <stdint.h>

namespace ambergris {
	class AgVoxelTreeRunTime;
	class AgVoxelContainer;

	class AgRegionVisibility
	{
	public:
		static bool isRegionVisible(uint8_t regionId);
		static bool isScanVisible(const AgVoxelTreeRunTime* treePtr, bool lightWeight);
		static bool isVoxelRegionVisible(const AgVoxelContainer& voxel);
		static bool isWholeVoxelVisible(const AgVoxelContainer& voxel);
	};
}
