#pragma once

#include "AgRawPointConverter.h"
#include "AgVoxelContainer.h"
#include "AgVoxelTreeRunTime.h"
#include "Resource/AgRenderResourceManager.h"

#include <common/RCTransform.h>
#include <common/RCVectorFwd.h>

namespace ambergris {

	class AgVoxelTreeRawPointConverter
	{
	public:
		static RealityComputing::Common::RCVector3d               rawPointToWorldCoord(const AgVoxelContainer *voxelPtr, const AgVoxelLeafNode& rhs);
		static RealityComputing::Common::RCVector3d               rawPointToWorldCoord(const AgVoxelContainer *voxelPtr, const AgVoxelTerrestialPoint* rhs);
		static RealityComputing::Common::RCVector3d               rawPointToWorldCoord(const AgVoxelContainer *voxelPtr, const AgVoxelLidarPoint* rhs);

		static RealityComputing::Common::RCVector3d               worldCoordToRawPoint(const AgVoxelContainer *voxelPtr, const RealityComputing::Common::RCVector3d& worldCoord);

		static RealityComputing::Common::RCVector3d               rawPointToScanCoord(const AgVoxelContainer *voxelPtr, const AgVoxelLeafNode& rhs);
		static RealityComputing::Common::RCVector3d               rawPointToScanCoord(const AgVoxelContainer *voxelPtr, const AgVoxelTerrestialPoint* rhs);
		static RealityComputing::Common::RCVector3d               rawPointToScanCoord(const AgVoxelContainer *voxelPtr, const AgVoxelLidarPoint* rhs);
		static RealityComputing::Common::RCVector3d               rawPointToScanCoord(const AgVoxelContainer *voxelPtr, const RealityComputing::Common::RCVector3d& rhs);
	};

	inline RealityComputing::Common::RCVector3d AgVoxelTreeRawPointConverter::rawPointToWorldCoord(const AgVoxelContainer *voxelPtr, const AgVoxelLeafNode& rhs)
	{
		auto curPoint = voxelPtr->m_parentTreePtr->getTransform() * rawPointToScanCoord(voxelPtr, rhs);
		return curPoint;
	}

	inline RealityComputing::Common::RCVector3d AgVoxelTreeRawPointConverter::rawPointToWorldCoord(const AgVoxelContainer *voxelPtr, const AgVoxelLidarPoint* rhs)
	{
		auto curPoint = voxelPtr->m_parentTreePtr->getTransform() * rawPointToScanCoord(voxelPtr, rhs);
		return curPoint;
	}

	inline RealityComputing::Common::RCVector3d AgVoxelTreeRawPointConverter::rawPointToWorldCoord(const AgVoxelContainer *voxelPtr, const AgVoxelTerrestialPoint* rhs)
	{
		auto curPoint = voxelPtr->m_parentTreePtr->getTransform() * rawPointToScanCoord(voxelPtr, rhs);
		return curPoint;
	}

	inline RealityComputing::Common::RCVector3d AgVoxelTreeRawPointConverter::worldCoordToRawPoint(const AgVoxelContainer *voxelPtr, const RealityComputing::Common::RCVector3d& worldCoord)
	{
		const auto scanCoord = voxelPtr->m_parentTreePtr->getTransform().getInverse(RealityComputing::Common::TransformType::Rigid) * worldCoord;
		AgBoundingbox* svoBbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(voxelPtr->m_svoBounds);
		if (!svoBbox)
			return RealityComputing::Common::RCVector3d();
		const auto rawCoord = AgRawPointConverter::scanCoordToRawPoint(svoBbox->m_bounds.getMin(), scanCoord);
		return rawCoord;
	}

	inline RealityComputing::Common::RCVector3d AgVoxelTreeRawPointConverter::rawPointToScanCoord(const AgVoxelContainer* voxelPtr, const RealityComputing::Common::RCVector3d& rhs)
	{
		AgBoundingbox* svoBbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(voxelPtr->m_svoBounds);
		if (!svoBbox)
			return RealityComputing::Common::RCVector3d();
		return AgRawPointConverter::rawPointToScanCoord(svoBbox->m_bounds.getMin(), rhs);
	}

	inline RealityComputing::Common::RCVector3d AgVoxelTreeRawPointConverter::rawPointToScanCoord(const AgVoxelContainer* voxelPtr, const AgVoxelLeafNode& rhs)
	{
		AgBoundingbox* svoBbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(voxelPtr->m_svoBounds);
		if (!svoBbox)
			return RealityComputing::Common::RCVector3d();
		return AgRawPointConverter::rawPointToScanCoord(svoBbox->m_bounds.getMin(), rhs);
	}

	inline RealityComputing::Common::RCVector3d AgVoxelTreeRawPointConverter::rawPointToScanCoord(const AgVoxelContainer *voxelPtr, const AgVoxelLidarPoint* rhs)
	{
		AgBoundingbox* svoBbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(voxelPtr->m_svoBounds);
		if (!svoBbox)
			return RealityComputing::Common::RCVector3d();
		return AgRawPointConverter::rawPointToScanCoord(svoBbox->m_bounds.getMin(), rhs);
	}

	inline RealityComputing::Common::RCVector3d AgVoxelTreeRawPointConverter::rawPointToScanCoord(const AgVoxelContainer *voxelPtr, const AgVoxelTerrestialPoint* rhs)
	{
		AgBoundingbox* svoBbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(voxelPtr->m_svoBounds);
		if (!svoBbox)
			return RealityComputing::Common::RCVector3d();
		return AgRawPointConverter::rawPointToScanCoord(svoBbox->m_bounds.getMin(), rhs);
	}
}