#include "AgVoxelTerrestialPoint.h"

#include <common/RCVector.h>

using namespace ambergris::RealityComputing::Common;

namespace ambergris {

	AgVoxelTerrestialPoint::AgVoxelTerrestialPoint()
	{
		m_data[0] = m_data[1] = m_data[2] = 0;
	}

	RCVector3f AgVoxelTerrestialPoint::getRawCoord() const
	{
		int x, y, z;
		x = m_data[0] & 0xFFF;
		y = (m_data[0] >> 12) & 0xFFF;
		z = (m_data[1]) & 0xFFF;

		RCVector3f coord;
		coord.x = static_cast<float>(x);
		coord.y = static_cast<float>(y);
		coord.z = static_cast<float>(z);

		return coord;
	}

	void AgVoxelTerrestialPoint::setRawCoord(const RCVector3f& val)
	{
		uint32_t offsetX = (uint32_t)val.x;
		uint32_t offsetY = (uint32_t)val.y;
		uint32_t offsetZ = (uint32_t)val.z;

		m_data[0] &= ~(0xFFF);        //clear x-coord bits
		m_data[0] |= offsetX;           //x coord bits

		m_data[0] &= ~(0xFFF << 12);  //clear y-coord bits
		m_data[0] |= (offsetY << 12); //y coord bits

		m_data[1] &= ~(0xFFF);        //clear z-coord bits
		m_data[1] |= offsetZ;           //z coord bits
	}

	RCVector4ub AgVoxelTerrestialPoint::getRGBA() const
	{
		RCVector4ub rgba;

		rgba.x = (m_data[2] >> 24) & 0xFF;
		rgba.y = (m_data[2] >> 16) & 0xFF;
		rgba.z = (m_data[2] >> 8) & 0xFF;
		rgba.w = (m_data[2] >> 0) & 0xFF;

		return rgba;
	}

	void AgVoxelTerrestialPoint::setRGBA(const RCVector4ub& val)
	{
		m_data[2] = 0;      //reset
		m_data[2] |= (val.x << 24);
		m_data[2] |= (val.y << 16);
		m_data[2] |= (val.z << 8);
		m_data[2] |= (val.w << 0);

	}

	RCVector3f AgVoxelTerrestialPoint::getNormal() const
	{
		return Math::NormalUtils::normalForIndex((m_data[1] >> 12) & 0x3FFF);
	}

	void AgVoxelTerrestialPoint::setNormalIndex(uint32_t normalIndex)
	{
		m_data[1] &= ~(0x3FFF << 12);         //clear normal bits
		m_data[1] |= (0x3FFF & normalIndex) << 12;    //set normal bits
	}


	int AgVoxelTerrestialPoint::getNormalIndex() const
	{
		return ((m_data[1] >> 12) & 0x3FFF);
	}

	void AgVoxelTerrestialPoint::setNormal(const RCVector3f& normal)
	{
		uint32_t index = Math::NormalUtils::indexForNormal(normal);
		m_data[1] &= ~(0x3FFF << 12);         //clear normal bits
		m_data[1] |= (0x3FFF & index) << 12;  //set normal bits
	}

	void AgVoxelTerrestialPoint::setLidarClassification(uint8_t/* lidarData*/)
	{

	}

	uint8_t AgVoxelTerrestialPoint::getLidarClassification() const
	{
		return 0;
	}

	uint8_t AgVoxelTerrestialPoint::getLayerInfo() const
	{
		return ((m_data[0] >> 24) & 0x7F);
	}

	/*uint8_t AgVoxelTerrestialPoint::getInheritRegionIndex(const VoxelContainer* parentVoxelContainer) const
	{
		if (parentVoxelContainer->isInheritRegionFlagValid())
		{
			return parentVoxelContainer->getInheritRegionIndex();
		}
		else
		{
			return getLayerInfo();
		}
	}*/

	void AgVoxelTerrestialPoint::setLayerInfo(uint8_t val)
	{

		uint32_t layerInfo = val;
		m_data[0] &= ~(0x7F << 24);           //clear layer bits
		m_data[0] |= (layerInfo << 24);       //set layer bits

	}

	/*bool AgVoxelTerrestialPoint::isDeleted(const VoxelContainer* parentVoxelContainer) const
	{
		uint8_t layerInfo = getInheritRegionIndex(parentVoxelContainer);
		return (alPointSelect::SELECTION_AND_DELETE_POINTS == layerInfo);
	}*/

	bool AgVoxelTerrestialPoint::getInTempSelection() const
	{
		return ((m_data[0] >> 31) & 0x1);
	}

	void AgVoxelTerrestialPoint::setInTempSelection(bool val)
	{
		m_data[0] &= ~(0x1u << 31);            //clear

		if (val)
			m_data[0] |= (0x1u << 31);     //set
		else
			m_data[0] |= (0x0 << 31);     //set 
	}

	bool AgVoxelTerrestialPoint::isFiltered() const
	{
		return bool((m_data[1] >> 26) & 0x01);
	}

	void AgVoxelTerrestialPoint::setFiltered(bool val)
	{
		if (val)
			m_data[1] |= (0x01 << 26);
		else
		{
			m_data[1] &= ~(0x01 << 26);
		}
	}

	bool AgVoxelTerrestialPoint::isClipped() const
	{
		return bool((m_data[1] >> 27) & 0x01);
	}
	
	void AgVoxelTerrestialPoint::setClipped(bool val)
	{
		if (val)
			m_data[1] |= ((0x01 & val) << 27);
		else
		{
			m_data[1] &= ~(0x01 << 27);
		}
	}
}