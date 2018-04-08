#pragma once

#include "AgVoxelLidarPoint.h"
#include "AgVoxelTerrestialPoint.h"

#include <common/RCTransform.h>
#include <common/RCVectorFwd.h>

namespace ambergris {
	class AgVoxelLeafNode;

	class AgRawPointConverter
	{
	public:
		static RealityComputing::Common::RCVector3d               rawPointToScanCoord(const RealityComputing::Common::RCVector3d& voxelOffset, const AgVoxelLeafNode& rhs);
		static RealityComputing::Common::RCVector3d               rawPointToScanCoord(const RealityComputing::Common::RCVector3d& voxelOffset, const AgVoxelTerrestialPoint* rhs);
		static RealityComputing::Common::RCVector3d               rawPointToScanCoord(const RealityComputing::Common::RCVector3d& voxelOffset, const AgVoxelLidarPoint* rhs);
		static RealityComputing::Common::RCVector3d               rawPointToScanCoord(const RealityComputing::Common::RCVector3d& voxelOffset, const RealityComputing::Common::RCVector3d& rhs);

		static RealityComputing::Common::RCVector3d               scanCoordToRawPoint(const RealityComputing::Common::RCVector3d& voxelOffset, const RealityComputing::Common::RCVector3d& scanCoord);

		static double getFromRawPointScale();

	private:
		static double getToRawPointScale();
	};

	inline RealityComputing::Common::RCVector3d AgRawPointConverter::rawPointToScanCoord(const RealityComputing::Common::RCVector3d& voxelOffset, const AgVoxelLeafNode& rhs)
	{
		return rawPointToScanCoord(voxelOffset, rhs.getRawOffsetFromBoundingBox().convertTo<double>());
	}

	inline RealityComputing::Common::RCVector3d AgRawPointConverter::rawPointToScanCoord(const RealityComputing::Common::RCVector3d& voxelOffset, const AgVoxelTerrestialPoint* rhs)
	{
		return rawPointToScanCoord(voxelOffset, rhs->getRawCoord().convertTo<double>());
	}

	inline RealityComputing::Common::RCVector3d AgRawPointConverter::rawPointToScanCoord(const RealityComputing::Common::RCVector3d& voxelOffset, const AgVoxelLidarPoint* rhs)
	{
		return rawPointToScanCoord(voxelOffset, rhs->getRawCoord().convertTo<double>());
	}

	inline RealityComputing::Common::RCVector3d AgRawPointConverter::rawPointToScanCoord(const RealityComputing::Common::RCVector3d& voxelOffset, const RealityComputing::Common::RCVector3d& rhs)
	{
		RealityComputing::Common::RCVector3d curPoint = (getFromRawPointScale() * rhs) + voxelOffset;
		return curPoint;
	}

	inline RealityComputing::Common::RCVector3d AgRawPointConverter::scanCoordToRawPoint(const RealityComputing::Common::RCVector3d& voxelOffset, const RealityComputing::Common::RCVector3d& scanCoord)
	{
		const auto rawCoord = getToRawPointScale() * (scanCoord - voxelOffset);
		return rawCoord;
	}

	inline double AgRawPointConverter::getFromRawPointScale()
	{
		return 0.001;
	}

	inline double AgRawPointConverter::getToRawPointScale()
	{
		return 1000;
	}
}
