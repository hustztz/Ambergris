#pragma once

#include "../AgObject.h"
#include "AgVoxelContainer.h"

#include <common/RCCode.h>
#include <common/RCTransform.h>

namespace ambergris {

	namespace RealityComputing {
		namespace Utility {
			class RCSpatialReference;
		}
	}

	class AgVoxelTreeRunTime : public AgObject
	{
	public:
		AgVoxelTreeRunTime();
		~AgVoxelTreeRunTime();

		//////////////////////////////////////////////////////////////////////////
		// \brief: Sets Voxel Tree Identifier
		//////////////////////////////////////////////////////////////////////////
		void                                    setId(const std::wstring& id);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Returns Voxel Tree Identifier
		//////////////////////////////////////////////////////////////////////////
		const std::wstring&                     getId() const;

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the SVO bounding box
		//////////////////////////////////////////////////////////////////////////
		AgBoundingbox::Handle					getSvoBounds() const;

		int                                     getAmountOfVoxelContainers() const;
		AgVoxelContainer*                       getVoxelContainerAt(int index);
		const AgVoxelContainer*                 getVoxelContainerAt(int index) const;

		bool                                    isClipFlag(ClipFlag rhs) const;
		bool                                    isInternalClipFlag(ClipFlag rhs) const;
		void                                    setExternalClipFlag(ClipFlag rhs);
		void                                    setInternalClipFlag(ClipFlag rhs);
		void                                    updateClipFlag();

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns if this scan is lidar data, and thus has a different
		//        vertex format
		//////////////////////////////////////////////////////////////////////////
		bool                                    isLidarData() const;

		//EngineSpatialFilter::FilterResult       getClipEngineFilterResult() const;
		//SDK::Interface::ARCSpatialFilter::FilterResult          getClipARCFilterResult() const;

		// \brief: set the region flag as invalid
		void                                    setRegionFlagInvalid();
		// \brief: set the region flag as valid
		void                                    setRegionFlagValid();
		// \brief: if a region flag is valid
		bool                                    isRegionFlagValid() const;
		// \brief: set the region index
		void                                    setRegionIndex(std::uint8_t index);
		// \brief: only valid when isRegionFlagValid returns true
		std::uint8_t                            getRegionIndex() const;

		// \brief: set the temp selection flag as invalid
		void                                    setInTempSelectionFlagInvalid();
		// \brief: set the temp selection flag as valid
		void                                    setInTempSelectionFlagValid();
		// \brief: if the temp selection flag is valid
		bool                                    isInTempSelectionFlagValid() const;
		// \brief: set whether inside/out side the temp selection
		void                                    setIsInTempSelection(bool flag);
		// \brief: only valid when
		bool                                    getIsInTempSelection() const;

		void transformPoint(const RealityComputing::Common::RCVector3d& pointIn, RealityComputing::Common::RCVector3d& pointOut) const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: change the coordinate system
		//////////////////////////////////////////////////////////////////////////
		RealityComputing::Common::RCCode setOriginalCoordinateSystem(const std::wstring& originalCS);
		RealityComputing::Common::RCCode setTargetCoordinateSystem(const std::wstring& targetCS, bool convertToMeter = false);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Returns the coordinate system
		//////////////////////////////////////////////////////////////////////////
		const std::wstring&                      getOriginalCoordinateSystem();
		const std::wstring&                      getTargetCoordinateSystem();

		//////////////////////////////////////////////////////////////////////////
		// \brief: return the Mentor coordinate transformation
		//////////////////////////////////////////////////////////////////////////
		RealityComputing::Utility::RCSpatialReference*       getMentorTransformation() const;

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the transformed bounding box
		//////////////////////////////////////////////////////////////////////////
		RealityComputing::Common::RCBox                       getTransformedBounds() const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: set global offset / geo-reference
		//////////////////////////////////////////////////////////////////////////
		void                          setGeoReference(double* geoReference);
		const double*                 getGeoReference() const;

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the original translation
		//////////////////////////////////////////////////////////////////////////
		const double*               getOriginalTranslation() const;

		//////////////////////////////////////////////////////////////////////////
		//\brief: Sets the original translation
		//////////////////////////////////////////////////////////////////////////
		void                        setOriginalTranslation(double* val);

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the original scale
		//////////////////////////////////////////////////////////////////////////
		const double*               getOriginalScale() const;

		//////////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////////
		void                       setOriginalScale(double* val);

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the original rotation
		//////////////////////////////////////////////////////////////////////////
		const double*               getOriginalRotation() const;

		//////////////////////////////////////////////////////////////////////////
		//\brief: set's the original rotation
		//////////////////////////////////////////////////////////////////////////
		void					  setOriginalRotation(double* val);

		// Additional transform, applied after calculation of m_transform from scale, rotation, and translation.
		void setAdditionalTransform(const RealityComputing::Common::RCTransform& addtl);

		void                     updateAll();

		virtual void			setGlobalPosition(double* val) override;
	protected:
		std::vector<AgVoxelContainer*>            m_leafNodeList;             //modified leaf node list, starts with containers that has occluders

	private:
		std::wstring                            mId;                //unique id

		ClipFlag                                m_clipFlag;
		ClipFlag                                mExternalClipFlag;
		ClipFlag                                mInternalClipFlag;

		AgBoundingbox::Handle			         m_svoBounds;

		std::wstring                            mOriginalCoordinateSystem;//RCS stored cs
		std::wstring                            mTargetCoordinateSystem;
		RealityComputing::Utility::RCSpatialReference*        mMentorTransformation;

		RealityComputing::Common::RCBox                       m_transformedBounds;
		RealityComputing::Common::RCVector3d                  m_scannerOrigin;

		RealityComputing::Common::RCBox                      mScanOriginalWorldBounds;

		RealityComputing::Common::RCScanProvider      m_pointCloudProvider;
		bool                                    mIsLidarData;
		bool                                    mLidarClassificationData[256];

		// \brief: unsigned short m_regionFlag
		/*
		*[0] whether in_temp_selection_flag is valid
		*[1] whether the voxel is inside temp selection or
		*    outside temp selection if in_temp_selection is valid
		*[2] whether in_region_flag is valid
		*[3...9] the region index if in region flag is valid
		*/
		unsigned short                          m_regionFlag;

		double								m_geoReference[3];
		double								m_originalTranslation[3];
		double								m_originalScale[3];
		double								m_originalRotation[3];

		double								m_finalTranslation[3];     // 'Final' translation has addt'l transform applied.

		// Additional transform, applied after calculation of m_transform from scale, rotation, and translation.
		RealityComputing::Common::RCTransform    m_additionalTransform;
	};
}