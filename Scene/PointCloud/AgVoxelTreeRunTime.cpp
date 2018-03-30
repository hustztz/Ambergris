#include "AgVoxelTreeRunTime.h"
#include "../AgSceneDatabase.h"
#include "Resource\AgRenderResourceManager.h"

#include <common/RCMemoryHelper.h>
#include <common/RCTransform.h>
#include <utility/RCSpatialReference.h>

using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility;

namespace ambergris {

	AgVoxelTreeRunTime::AgVoxelTreeRunTime()
		: mMentorTransformation(NULL)
		, mIsLidarData(false)
	{
		m_geoReference[0] = m_geoReference[1] = m_geoReference[2] = 0.0;
		m_originalTranslation[0] = m_originalTranslation[1] = m_originalTranslation[2] = 0.0;
		m_originalRotation[0] = m_originalRotation[1] = m_originalRotation[2] = 0.0;
		m_originalScale[0] = m_originalScale[1] = m_originalScale[2] = 1.0;
		// initialize mLidarClassificationData
		for (int i = 0; i < 256; i++)
			mLidarClassificationData[i] = false;
	}

	AgVoxelTreeRunTime::~AgVoxelTreeRunTime()
	{
		DeletePtr(mMentorTransformation);
	}

	const std::wstring& AgVoxelTreeRunTime::getId() const
	{
		return mId;
	}

	void AgVoxelTreeRunTime::setId(const std::wstring& id)
	{
		mId = id;
	}

	AgBoundingbox::Handle AgVoxelTreeRunTime::getSvoBounds() const
	{
		return m_svoBounds;
	}

	void AgVoxelTreeRunTime::setGeoReference(double* geoReference)
	{
		m_geoReference[0] = geoReference[0];
		m_geoReference[1] = geoReference[1];
		m_geoReference[2] = geoReference[2];
	}

	const double* AgVoxelTreeRunTime::getGeoReference() const
	{
		return m_geoReference;
	}

	void AgVoxelTreeRunTime::transformPoint(const RCVector3d& pointIn, RCVector3d& pointOut) const
	{
		const AgCacheTransform* cacheTransform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
		RCTransform transform;
		transform.fromColumnMajor(cacheTransform->m_mtx);
		pointOut = transform * pointIn;
		RCSpatialReference* pTransformation = getMentorTransformation();
		if (pTransformation != nullptr)
		{
			pointOut = Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.globalToWorld(pointOut);
			pTransformation->transform(pointOut);
			pointOut = Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.worldToGlobal(pointOut);
		}
	}

	AgVoxelContainer* AgVoxelTreeRunTime::getVoxelContainerAt(int index)
	{
		if (index >= 0 && index < static_cast<int>(m_leafNodeList.size()))
			return m_leafNodeList[index];
		else
			return NULL;
	}

	const AgVoxelContainer* AgVoxelTreeRunTime::getVoxelContainerAt(int index) const
	{
		if (index >= 0 && index < static_cast<int>(m_leafNodeList.size()))
			return m_leafNodeList[index];
		else
			return NULL;
	}

	int AgVoxelTreeRunTime::getAmountOfVoxelContainers() const
	{
		return (int)m_leafNodeList.size();
	}

	bool AgVoxelTreeRunTime::isClipFlag(ClipFlag rhs) const
	{
		return m_clipFlag == rhs;
	}

	bool AgVoxelTreeRunTime::isInternalClipFlag(ClipFlag rhs) const
	{
		return mInternalClipFlag == rhs;
	}

	void AgVoxelTreeRunTime::setExternalClipFlag(ClipFlag rhs)
	{
		mExternalClipFlag = rhs;
		updateClipFlag();
	}

	void AgVoxelTreeRunTime::setInternalClipFlag(ClipFlag rhs)
	{
		mInternalClipFlag = rhs;
		updateClipFlag();
	}

	void AgVoxelTreeRunTime::updateClipFlag()
	{
		m_clipFlag = AgVoxelContainer::intersectClipFlag(mInternalClipFlag, mExternalClipFlag);
	}

	void AgVoxelTreeRunTime::setRegionFlagInvalid()
	{
		m_regionFlag &= ~(0x1 << 2);
	}
	void AgVoxelTreeRunTime::setRegionFlagValid()
	{
		m_regionFlag |= 0x1 << 2;
	}
	bool AgVoxelTreeRunTime::isRegionFlagValid() const
	{
		return (m_regionFlag & 0x1 << 2) != 0;
	}
	void AgVoxelTreeRunTime::setRegionIndex(std::uint8_t index)
	{
		setRegionFlagValid();
		m_regionFlag &= ~(0x7F << 3);
		m_regionFlag |= index << 3;
	}
	std::uint8_t AgVoxelTreeRunTime::getRegionIndex() const
	{
		return static_cast<std::uint8_t>((m_regionFlag >> 3) & 0x7F);
	}
	void AgVoxelTreeRunTime::setInTempSelectionFlagInvalid()
	{
		m_regionFlag &= ~(0x1);
	}
	void AgVoxelTreeRunTime::setInTempSelectionFlagValid()
	{
		m_regionFlag |= 0x1;
	}
	bool AgVoxelTreeRunTime::isInTempSelectionFlagValid() const
	{
		return m_regionFlag & 0x1;
	}
	void AgVoxelTreeRunTime::setIsInTempSelection(bool flag)
	{
		setInTempSelectionFlagValid();
		m_regionFlag &= ~(0x1 << 1);
		m_regionFlag |= flag << 1;
	}
	// \brief: only valid when 
	bool AgVoxelTreeRunTime::getIsInTempSelection() const
	{
		return (m_regionFlag >> 1) & 0x1;
	}

	bool AgVoxelTreeRunTime::isLidarData() const
	{
		return (m_pointCloudProvider == RCScanProvider::PROVIDER_LAS_DATA || mIsLidarData);
	}

	RCCode AgVoxelTreeRunTime::setOriginalCoordinateSystem(const std::wstring& originalCS)
	{
		if (!originalCS.empty())
		{
			RCCode result = RCSpatialReferenceUtils::isValidCoordinateSystem(originalCS.c_str());
			if (result != rcOK)
			{
				return result;
			}

			mOriginalCoordinateSystem = originalCS;
		}
		return rcOK;
	}

	RCCode AgVoxelTreeRunTime::setTargetCoordinateSystem(const std::wstring& targetCS, bool convertToMeter /*= false*/)
	{
		if (mOriginalCoordinateSystem.empty() || targetCS.empty())
		{
			return rcUnknownCoordSystem;
		}

		//only for lidar data
		if (!isLidarData())
		{
			return rcNotLidarData;
		}

		RCCode result = RCSpatialReferenceUtils::isValidCoordinateSystem(targetCS.c_str());
		if (result != rcOK)
		{
			return result;
		}

		if (mMentorTransformation == NULL)
		{
			mMentorTransformation = new RCSpatialReference();
		}

		if (mMentorTransformation == NULL)
		{
			return rcFailed;
		}

		result = mMentorTransformation->setCoordinateSystem(mOriginalCoordinateSystem.c_str(), targetCS.c_str(), convertToMeter);
		if (result != rcOK)
		{
			return result;
		}

		mTargetCoordinateSystem = targetCS;

		return rcOK;
	}

	RCSpatialReference* AgVoxelTreeRunTime::getMentorTransformation() const
	{
		return mMentorTransformation;
	}

	const std::wstring& AgVoxelTreeRunTime::getOriginalCoordinateSystem()
	{
		return mOriginalCoordinateSystem;
	}

	const std::wstring& AgVoxelTreeRunTime::getTargetCoordinateSystem()
	{
		if (mTargetCoordinateSystem.empty())
		{
			return mOriginalCoordinateSystem;
		}

		return mTargetCoordinateSystem;
	}

	const double* AgVoxelTreeRunTime::getOriginalTranslation() const
	{
		return m_originalTranslation;
	}

	void AgVoxelTreeRunTime::setOriginalTranslation(double* val)
	{
		m_originalTranslation[0] = val[0];
		m_originalTranslation[1] = val[1];
		m_originalTranslation[2] = val[2];
	}

	const double* AgVoxelTreeRunTime::getOriginalScale() const
	{
		return m_originalScale;
	}

	void AgVoxelTreeRunTime::setOriginalScale(double* val)
	{
		m_originalScale[0] = val[0];
		m_originalScale[1] = val[1];
		m_originalScale[2] = val[2];
	}

	const double* AgVoxelTreeRunTime::getOriginalRotation() const
	{
		return m_originalRotation;
	}

	void AgVoxelTreeRunTime::setOriginalRotation(double* val)
	{
		m_originalRotation[0] = val[0];
		m_originalRotation[1] = val[1];
		m_originalRotation[2] = val[2];
	}

	void AgVoxelTreeRunTime::setAdditionalTransform(const RCTransform& addtl)
	{
		m_additionalTransform = addtl;

		AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
		if (!transform)
			return;

		RCVector3d translation(transform->m_mtx[12], transform->m_mtx[13], transform->m_mtx[14]);
		RCVector3d finalTranslation = m_additionalTransform * translation;
		m_finalTranslation[0] = finalTranslation.x;
		m_finalTranslation[1] = finalTranslation.y;
		m_finalTranslation[2] = finalTranslation.z;
	}

	/*virtual*/
	void AgVoxelTreeRunTime::setGlobalPosition(double* val)
	{
		AgObject::setGlobalPosition(val);

		AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
		if (!transform)
			return;

		RCVector3d translation(transform->m_mtx[12], transform->m_mtx[13], transform->m_mtx[14]);
		RCVector3d finalTranslation = m_additionalTransform * translation;
		m_finalTranslation[0] = finalTranslation.x;
		m_finalTranslation[1] = finalTranslation.y;
		m_finalTranslation[2] = finalTranslation.z;
	}

	RCBox AgVoxelTreeRunTime::getTransformedBounds() const
	{
		if (mMentorTransformation == NULL)
		{
			return m_transformedBounds;
		}
		else
		{
			//transform the bound with Mentor
			const RCBox worldBounds = Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.globalToWorld(m_transformedBounds);
			const RCBox mentorTransformedBounds = mMentorTransformation->transformBounds(worldBounds);
			const RCBox projGlobalBounds = Singleton<AgSceneDatabase>::instance().mWorldCoordInfo.worldToGlobal(mentorTransformedBounds);
			return projGlobalBounds;
		}
	}

	void AgVoxelTreeRunTime::updateAll()
	{
		buildTransform();
		m_transformedBounds = getBoundingBox().getTransformed(getTransform());

		if (mScanOriginalWorldBounds.isEmpty())
		{
			mScanOriginalWorldBounds = m_transformedBounds;
		}

		for (auto& leafNode : m_leafNodeList)
		{
			leafNode->setTransformedSVOBound(leafNode->m_svoBounds.getTransformed(getTransform()));
			leafNode->setTransformedCenter(leafNode->getTransformedSVOBound().getCenter());
			leafNode->setTransformedNodeRadius(leafNode->getTransformedSVOBound().getMax().x - leafNode->getTransformedSVOBound().getMin().x);
		}
	}
}