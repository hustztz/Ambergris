#pragma once
#include <common/RCVectorFwd.h>

#include <stdint.h>

//////////////////////////////////////////////////////////////////////////
// \brief: VoxelTerrestialPoint  Compressed format for terrestrial based data
//////////////////////////////////////////////////////////////////////////
/*
VoxelTerrestialPoint ( 32*3 bits total )
//UInt #1
[0.....11]          x Coordinates
[12....23]          y Coordinates
[24....30]          layer
[31]                Is in temp selection

//UInt #2
[0....11]           z Coordinates RGBA
[12...25]           Normal index
[26]                Filtered : 1 filtered, 0 not filtered
[27]                Crop: 1 clipped, 0 not clipped
[28...31]           Classification(PRIMITIVES)  --> Noise, Plane, Cylinder, Sphere //1, 2, 3, 4

//UInt #3
[0....7]            R
[8...15]            G
[16...23]           B
[24...31]           A
*/

namespace ambergris {

	class VoxelContainer;

	struct AgVoxelTerrestialPoint
	{
		AgVoxelTerrestialPoint();

		RealityComputing::Common::RCVector3f  getRawCoord() const;
		void                                    setRawCoord(const RealityComputing::Common::RCVector3f& val);

		RealityComputing::Common::RCVector4ub                     getRGBA() const;
		void                                    setRGBA(const RealityComputing::Common::RCVector4ub& val);

		RealityComputing::Common::RCVector3f                      getNormal() const;
		void                                    setNormal(const RealityComputing::Common::RCVector3f& normal);

		void                                    setNormalIndex(uint32_t normalIndex);
		int                                     getNormalIndex() const;

		void                                    setLidarClassification(uint8_t lidarData);
		uint8_t									getLidarClassification() const;

		uint8_t									getLayerInfo() const;
		void                                    setLayerInfo(uint8_t val);

		//uint8_t									getInheritRegionIndex(const VoxelContainer* parentVoxelContainer) const;

		bool                                    getInTempSelection() const;
		void                                    setInTempSelection(bool val);

		//virtual std::uint8_t                  getPrimitiveClassification( ) const;
		//virtual void                          setPrimitiveClassification( std::uint8_t val );

		void                                    setFiltered(bool val);
		bool                                    isFiltered() const;

		void                                    setClipped(bool val);
		bool                                    isClipped() const;

		//bool                                    isDeleted(const VoxelContainer* parentVoxelContainer) const;

		uint32_t								m_data[3];
	};
}
