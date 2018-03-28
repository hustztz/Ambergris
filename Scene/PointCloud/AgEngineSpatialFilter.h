#pragma once

#include <vector>

#include <common/RCVectorFwd.h>

namespace ambergris {

	namespace RealityComputing {
		namespace Common {

			class RCTransform;
			struct RCBox;

		}
	}

	class AgVoxelTreeRunTime;
	class AgVoxelContainer;
	class PointClipManager;
	struct alTriangleSelectionVolume;

	// \brief: EngineSpatialFilter is prepared for the region/selction/clip logic
	//         in StudioCloud Project
	class AgEngineSpatialFilter /*: public Interface::ARCSpatialFilter*/
	{
	public:
		enum FilterResult
		{
			FILTER_INSIDE = 0,  // entirely inside the filter
			FILTER_OUTSIDE,     // entirely outside
			FILTER_INTERSECTS   // partially inside
		};
	public:
		virtual ~AgEngineSpatialFilter() = 0;
		// \brief: use the transformed bound of treePtr
		virtual FilterResult         checkScan(const RealityComputing::Common::RCBox& bound, const AgVoxelTreeRunTime* treePtr) const = 0;
		// \brief: use the transformed bound of voxelPtr
		virtual FilterResult         checkVoxelContainer(const RealityComputing::Common::RCBox& bound, const AgVoxelContainer* voxelPtr) const = 0;
		virtual FilterResult         checkPoint(const RealityComputing::Common::RCVector3d& rhs, const AgVoxelContainer* voxelBelonger) const = 0;
		virtual AgEngineSpatialFilter* transformFilter(const RealityComputing::Common::RCTransform& transform4x4) const = 0;
		virtual AgEngineSpatialFilter* clone() const = 0;
		virtual void                 free() = 0;

		virtual void                 checkPointList(const RealityComputing::Common::RCVector3d* pointList,
			unsigned int pointNum, FilterResult *outResults,
			const RealityComputing::Common::RCBox& rhs, const AgVoxelContainer* voxelBelonger) const;
		virtual bool                 isCheckPointListOptimized() const;

		//capabilities added to assist with visualizing operations/filters
		virtual bool hasTriangleSelectionVolume() const;
		virtual std::vector<alTriangleSelectionVolume>  getTriangleSelectionVolume() const;
		virtual bool hasClipManager() const;
		virtual const PointClipManager*  getClipManager() const;
	};
}