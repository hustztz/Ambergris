#include "AgRegionVisibility.h"

using namespace ambergris;

bool AgRegionVisibility::isRegionVisible(uint8_t regionId)
{
	//TODO
	return true;
}

bool AgRegionVisibility::isVoxelRegionVisible(const AgVoxelContainer& voxel)
{
	/*if (voxel.isInheritRegionFlagValid())
	{
		std::uint8_t regionId = voxel.getInheritRegionIndex();
		return isRegionVisible(regionId, pRegions);
	}*/

	return true;
}