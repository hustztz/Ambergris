#pragma once

#include <common/RCVector.h>

#include <vector>

namespace ambergris {

	/// this struct contains properties of alSvoTree
	struct AgVoxelTreeNode
	{
		AgVoxelTreeNode()
		{
			lidarClassifications = std::vector<std::uint16_t>();
		}
		bool                            visible;
		bool                            registered;
		RealityComputing::Common::RCVector4f              color;
		bool                            selected;
		RealityComputing::Common::RCVector3d              translation;
		RealityComputing::Common::RCVector3d              rotation;
		RealityComputing::Common::RCVector3d              scale;
		std::wstring                    nodeName;
		std::wstring                    group;
		std::wstring                    path;
		std::wstring                    relativePath;
		std::wstring                    id;
		std::int64_t                    numPoints;
		bool                            hasRGB;
		bool                            hasNormals;
		bool                            hasIntensity;
		bool                            validFile;
		bool                            normalizeIntensity;
		float                           maxIntensity;
		float                           minIntensity;
		int                             rangeImageWidth;
		int                             rangeImageHeight;
		bool                            isLidarData;
		std::vector<std::uint16_t>  lidarClassifications;
	};
}