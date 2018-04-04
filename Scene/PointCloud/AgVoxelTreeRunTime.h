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

	struct AgVoxelTreeNode;

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
		const std::wstring&                      getOriginalCoordinateSystem() const;
		const std::wstring&                      getTargetCoordinateSystem() const;

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
		void                          setGeoReference(const RealityComputing::Common::RCVector3d& geoReference) { m_geoReference = geoReference; }
		const RealityComputing::Common::RCVector3d&      getGeoReference() const { return m_geoReference; }

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the original translation
		//////////////////////////////////////////////////////////////////////////
		const RealityComputing::Common::RCVector3d&   getOriginalTranslation() const { return m_originalTranslation; }

		//////////////////////////////////////////////////////////////////////////
		//\brief: Sets the original translation
		//////////////////////////////////////////////////////////////////////////
		void                        setOriginalTranslation(const RealityComputing::Common::RCVector3d& val) { m_originalTranslation = val; }

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the original scale
		//////////////////////////////////////////////////////////////////////////
		const RealityComputing::Common::RCVector3d&  getOriginalScale() const { return m_originalScale; }

		//////////////////////////////////////////////////////////////////////////
		//
		//////////////////////////////////////////////////////////////////////////
		void                       setOriginalScale(const RealityComputing::Common::RCVector3d& val) { m_originalScale = val; }

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the original rotation
		//////////////////////////////////////////////////////////////////////////
		const RealityComputing::Common::RCVector3d&   getOriginalRotation() const { return m_originalRotation; }

		//////////////////////////////////////////////////////////////////////////
		//\brief: set's the original rotation
		//////////////////////////////////////////////////////////////////////////
		void					  setOriginalRotation(const RealityComputing::Common::RCVector3d& val) { m_originalRotation = val; }

		// Additional transform, applied after calculation of m_transform from scale, rotation, and translation.
		void setAdditionalTransform(const RealityComputing::Common::RCTransform& addtl) { m_additionalTransform = addtl; }

		void                     updateAll();

		RealityComputing::Common::RCVector3d	getFinalTranslation() const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: Returns if this scan has time stamp information 
		//////////////////////////////////////////////////////////////////////////
		bool                                    hasTimeStamp() const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: Sets if this scan will use the transform from RCS headers
		//////////////////////////////////////////////////////////////////////////
		void                                    setUseFileHeaderTransform(bool val) { m_useFileHeaderTransform = val; }
		//////////////////////////////////////////////////////////////////////////
		//\brief: Sets the search paths
		//////////////////////////////////////////////////////////////////////////
		void                                    setSearchPaths(const std::vector<std::wstring>& searchPaths) { mSearchPaths = searchPaths; }

		void                                  setProjectDirectory(const std::wstring& projectDir);

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns the file name
		//////////////////////////////////////////////////////////////////////////
		const std::wstring&                     getFileName() const;

		//////////////////////////////////////////////////////////////////////////
		//\brief: Check if this scan has segmentation information
		//////////////////////////////////////////////////////////////////////////
		void                                    checkSegmentInfo();

		//////////////////////////////////////////////////////////////////////////
		//\brief: Returns if this scan has segmentation information
		//////////////////////////////////////////////////////////////////////////
		bool                                    hasSegmentInfo() const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: Set's a file Chunk, return true upon find
		//////////////////////////////////////////////////////////////////////////
		bool                                    getFileChunk(std::uint32_t chunkId, OctreeFileChunk& chunk) const;

		/// read file header, return rcOK if the file format is latest, return rsLegacyFileFormat if the file format is out of date,
		/// returns rsUnknownFileFormat if the file is not a valid K2 file, returns rsDeprecatedFileFormat if the file format is too old to support
		RealityComputing::Common::RCCode readFileHeader(const std::wstring& fileName, unsigned int* checksum = nullptr);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Open a point cloud file from disk, returns rcOK if successful,
		//      returns rsFileNotExist if there are files not found,
		//      returns rsLegacyFileFormat if there are legacy files,
		//      returns rsfilecorrupt if the file is corrupt
		//////////////////////////////////////////////////////////////////////////
		RealityComputing::Common::RCCode loadFromMedium(const std::wstring& projectFile, const std::wstring& fileName, bool isLightWeight);
		//////////////////////////////////////////////////////////////////////////
		// \brief: Parse Segment Data
		//////////////////////////////////////////////////////////////////////////
		bool                                    parseSegmentData(const std::wstring& fileName);

		//////////////////////////////////////////////////////////////////////////
		// \brief: parse time stamp data
		//////////////////////////////////////////////////////////////////////////
		bool                                    parseTimeStampData(const std::wstring& fileName);

		//////////////////////////////////////////////////////////////////////////
		//\brief: Parse Umbrella octree data
		//////////////////////////////////////////////////////////////////////////
		bool                                    parseUmbrellaOctreeData(const std::wstring& fileName);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Parse Other data
		//////////////////////////////////////////////////////////////////////////
		bool                                    parseOtherData(const std::wstring& fileName);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Parse Voxel/Points Data
		//////////////////////////////////////////////////////////////////////////
		bool                                    parseVoxelData(const std::wstring& fileName);
		/// override method, for reattach scans
		bool				onLoad(const std::wstring& projectFile, const AgVoxelTreeNode& treeNode, bool isLightWeight);
	protected:
		/// internal use only, check if the file format is legacy
		bool                                    _IsLegacyFileFormat(std::uint32_t majorVersion, std::uint32_t minorVersion) const;
	protected:
		std::vector<AgVoxelContainer*>            m_leafNodeList;             //modified leaf node list, starts with containers that has occluders

	private:
		std::wstring                            m_fileName;         //file path
		std::wstring                            mRelativeFileName;
		std::wstring                            mProjectDirectory;
		std::wstring                            mId;                //unique id
		bool                                    m_useFileHeaderTransform;
		std::vector<std::wstring>               mSearchPaths;

		std::vector<OctreeFileChunk>			m_chunkList;
		int                                     m_octreeFlags;

		bool                                    mNormalizeIntensity;
		float                                   mMaxIntensity;
		float                                   mMinIntensity;
		int                                     mRangeImageHeight;
		int                                     mRangeImageWidth;

		double                                  mRangeImageHorizontalStartAngle;
		double                                  mRangeImageHorizontalEndAngle;
		double                                  mRangeImageVerticalStartAngle;
		double                                  mRangeImageVerticalEndAngle;

		ClipFlag                                m_clipFlag;
		ClipFlag                                mExternalClipFlag;
		ClipFlag                                mInternalClipFlag;

		bool                                    mHasRGB;
		bool                                    mHasNormals;
		bool                                    mHasIntensity;
		bool                                    mHasSegmentInfo;
		int                                     mAmountOfSegments;
		bool                                    mHasCheckedSegmentInfo;
		int                                     m_amountOfChunks;

		AgBoundingbox::Handle			         m_svoBounds;

		std::wstring                            mOriginalCoordinateSystem;//RCS stored cs
		std::wstring                            mTargetCoordinateSystem;
		RealityComputing::Utility::RCSpatialReference*        mMentorTransformation;

		RealityComputing::Common::RCBox                       m_transformedBounds;
		RealityComputing::Common::RCVector3d                  m_scannerOrigin;

		//RealityComputing::Common::RCBox                      mScanOriginalWorldBounds;

		RealityComputing::Common::RCScanProvider      m_pointCloudProvider;
		std::uint64_t                                           m_totalAmountOfPoints;
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

		RealityComputing::Common::RCVector3d								m_geoReference;
		RealityComputing::Common::RCVector3d								m_originalTranslation;
		RealityComputing::Common::RCVector3d								m_originalScale;
		RealityComputing::Common::RCVector3d								m_originalRotation; //in euler angles

		// Additional transform, applied after calculation of m_transform from scale, rotation, and translation.
		RealityComputing::Common::RCTransform    m_additionalTransform;

		RealityComputing::Common::RCVector4f              m_nodeColor;
		bool									m_registered;
	};
}