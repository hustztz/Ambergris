#include "AgVoxelTreeRunTime.h"
#include "AgProjectInformation.h"
#include "AgOctreeDefinitions.h"
#include "../AgSceneDatabase.h"
#include "Resource\AgRenderResourceManager.h"

#include <common/RCMemoryHelper.h>
#include <common/RCMemoryMapFile.h>
#include <common/RCTransform.h>
#include <utility/RCSpatialReference.h>
#include <utility/RCFilesystem.h>
#include <utility/RCStringUtils.h>
#include <io/RCChecksum.h>
#include <import/OctreeIntermediateNode.h>

#include <assert.h>
#include <algorithm>

using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility;
using namespace ambergris::RealityComputing::Import;

namespace ambergris {

	AgVoxelTreeRunTime::AgVoxelTreeRunTime()
		: mMentorTransformation(NULL)
		, mIsLidarData(false)
		, m_useFileHeaderTransform(true)
		, mHasRGB(false)
		, mHasNormals(false)
		, mHasIntensity(false)
		, mHasSegmentInfo(false)
		, mAmountOfSegments(0)
		, mHasCheckedSegmentInfo(false)
		, m_amountOfChunks(0)
		, mNormalizeIntensity(true)
		, mMaxIntensity(255)
		, mMinIntensity(0)
		, mRangeImageHeight(0)
		, mRangeImageWidth(0)
		, m_totalAmountOfPoints(0)
		, m_registered(false)
	{
		mRelativeFileName = L"";
		mProjectDirectory = L"";

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

	void AgVoxelTreeRunTime::transformPoint(const RCVector3d& pointIn, RCVector3d& pointOut) const
	{
		const AgCacheTransform* cacheTransform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
		if (!cacheTransform || AgCacheTransform::kInvalidHandle == cacheTransform->m_handle)
			return;

		pointOut = cacheTransform->m_transform * pointIn;
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

	const std::wstring& AgVoxelTreeRunTime::getOriginalCoordinateSystem() const
	{
		return mOriginalCoordinateSystem;
	}

	const std::wstring& AgVoxelTreeRunTime::getTargetCoordinateSystem() const
	{
		if (mTargetCoordinateSystem.empty())
		{
			return mOriginalCoordinateSystem;
		}

		return mTargetCoordinateSystem;
	}

	RCVector3d	AgVoxelTreeRunTime::getFinalTranslation() const
	{
		AgCacheTransform* transform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
		if (!transform || AgCacheTransform::kInvalidHandle == transform->m_handle)
			return RCVector3d();

		RCVector3d finalTranslation = m_additionalTransform * transform->m_transform.getTranslation();
		return finalTranslation;
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

	namespace
	{
		template <class Iter>
		bool hasStructuredChunk(const Iter& start, const Iter& end)
		{
			return end != std::find_if(start, end, [](const OctreeFileChunk& ofc) -> bool {
				return (ofc.m_chunkId == ENUM_FILE_CHUNK_ID_SEGMENT_INFO); })
				&& end != std::find_if(start, end, [](const OctreeFileChunk& ofc) -> bool {
					return (ofc.m_chunkId == ENUM_FILE_CHUNK_ID_VOXEL_SEGMENT_DATA); });
		}

		template<class Iter>
		bool hasTimeStampChunk(const Iter& start, const Iter& end)
		{
			return end != std::find_if(start, end, [](const OctreeFileChunk& ofc) -> bool {
				return (ofc.m_chunkId == ENUM_FILE_CHUNK_ID_TIME_STAMP);
			});
		}
	}

	bool AgVoxelTreeRunTime::hasTimeStamp() const
	{
		return (m_octreeFlags & ENUM_OCTREE_FLAGS_HAS_TIME_STAMP) && hasTimeStampChunk(m_chunkList.begin(), m_chunkList.end());
	}

	void AgVoxelTreeRunTime::updateAll()
	{
		AgBoundingbox* bbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(m_bbox);
		if (bbox)
		{
			const AgCacheTransform* cacheTransform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
			if (cacheTransform)
			{
				RCVector3d minPoint((double)bbox->m_aabb.m_min[0], (double)bbox->m_aabb.m_min[1], (double)bbox->m_aabb.m_min[2]);
				RCVector3d maxPoint((double)bbox->m_aabb.m_max[0], (double)bbox->m_aabb.m_max[1], (double)bbox->m_aabb.m_max[2]);
				m_transformedBounds = RCBox(minPoint, maxPoint);
				m_transformedBounds.transform(cacheTransform->m_transform);
			}
		}

		/*if (mScanOriginalWorldBounds.isEmpty())
		{
			mScanOriginalWorldBounds = m_transformedBounds;
		}*/

		for (auto& leafNode : m_leafNodeList)
		{
			AgBoundingbox* svoBbox = Singleton<AgRenderResourceManager>::instance().m_bboxManager.get(leafNode->m_svoBounds);
			if (!svoBbox)
				continue;
			const AgCacheTransform* cacheTransform = Singleton<AgRenderResourceManager>::instance().m_transforms.get(m_global_transform_h);
			if (!cacheTransform)
				continue;

			RCVector3d minPoint((double)svoBbox->m_aabb.m_min[0], (double)svoBbox->m_aabb.m_min[1], (double)svoBbox->m_aabb.m_min[2]);
			RCVector3d maxPoint((double)svoBbox->m_aabb.m_max[0], (double)svoBbox->m_aabb.m_max[1], (double)svoBbox->m_aabb.m_max[2]);
			RCBox svoBounds = RCBox(minPoint, maxPoint);
			svoBounds.transform(cacheTransform->m_transform);

			leafNode->setTransformedSVOBound(svoBounds);
			leafNode->setTransformedCenter(leafNode->getTransformedSVOBound().getCenter());
			leafNode->setTransformedNodeRadius(leafNode->getTransformedSVOBound().getMax().x - leafNode->getTransformedSVOBound().getMin().x);
		}
	}

	void AgVoxelTreeRunTime::setProjectDirectory(const std::wstring& projectDir)
	{
		mProjectDirectory = Filesystem::isDirectory(projectDir)
			? projectDir
			: Filesystem::parentPath(projectDir);
	}

	const std::wstring& AgVoxelTreeRunTime::getFileName() const
	{
		return m_fileName;
	}

	bool AgVoxelTreeRunTime::getFileChunk(std::uint32_t chunkId, OctreeFileChunk& chunk) const
	{
		for (size_t i = 0; i < m_chunkList.size(); i++)
		{
			if (m_chunkList[i].m_chunkId == chunkId)
			{
				chunk = m_chunkList[i];
				return true;
			}
		}
		return false;
	}

	bool AgVoxelTreeRunTime::_IsLegacyFileFormat(std::uint32_t majorVersion, std::uint32_t minorVersion) const
	{
		if (majorVersion == 3 && minorVersion == 1)
			return true;

		return false;
	}

	RCCode AgVoxelTreeRunTime::readFileHeader(const std::wstring& fileName, unsigned int* checksum)
	{
		// check file size
		long long fileSize = Filesystem::fileSize(fileName);
		if (fileSize <= static_cast<long long>(sizeof(OctreeFileHeader)))
			return rcUnknownFileFormat;

		OctreeFileHeader fileHeader;
		RCMemoryMapFile memmapFile(fileName.c_str());
		if (!memmapFile.createFileHandleOnlyRead())
			return rcUnknownFileFormat;
		memmapFile.setFilePointer(0);
		if (!memmapFile.readFile(&fileHeader, sizeof(OctreeFileHeader)))
			return rcUnknownFileFormat;
		if (fileHeader.m_magicWord[0] != 'A' || fileHeader.m_magicWord[1] != 'D' ||
			fileHeader.m_magicWord[2] != 'O' || fileHeader.m_magicWord[3] != 'C' ||
			fileHeader.m_magicWord[4] != 'T')
		{
			return rcUnknownFileFormat;
		}

		// check version    
		bool legacyFileFormat = false;
		if (_IsLegacyFileFormat(fileHeader.m_majorVersion, fileHeader.m_minorVersion == 1))
		{
			legacyFileFormat = true;
		}
		else if (fileHeader.m_majorVersion > MAJOR_VERSION)
		{
			return rcFutureFileFormat;
		}
		else if (fileHeader.m_majorVersion != MAJOR_VERSION || fileHeader.m_minorVersion != MINOR_VERSION)
		{
			return rcDeprecatedFileFormat;
		}

		if (checksum)
		{
			*checksum = RealityComputing::IO::RCChecksum::checksum_buffer(&fileHeader, sizeof(OctreeFileHeader));
		}

		m_fileName = fileName;

		if (m_useFileHeaderTransform)
		{
			m_scannerOrigin = fileHeader.m_scannerOrigin;
			Singleton<AgRenderResourceManager>::instance().m_transforms.
				setTransform(m_global_transform_h, fileHeader.m_translation, fileHeader.m_rotation, fileHeader.m_scale);
		}

		m_originalRotation = fileHeader.m_rotation;
		m_originalScale = fileHeader.m_scale;
		m_originalTranslation = fileHeader.m_translation;

		m_octreeFlags = fileHeader.m_octreeFlags;
		m_pointCloudProvider = fileHeader.m_pointCloudProvider;
		m_totalAmountOfPoints = fileHeader.m_totalAmountOfPoints;

		float boundsMin[3] = { (float)fileHeader.m_scanBounds.getMin().x, (float)fileHeader.m_scanBounds.getMin().y, (float)fileHeader.m_scanBounds.getMin().z };
		float boundsMax[3] = { (float)fileHeader.m_scanBounds.getMax().x, (float)fileHeader.m_scanBounds.getMax().y, (float)fileHeader.m_scanBounds.getMax().z };
		Singleton<AgRenderResourceManager>::instance().m_bboxManager.
			setBoundingBox(m_bbox, boundsMin, boundsMax);
		float svoBoundsMin[3] = { (float)fileHeader.m_svoBounds.getMin().x, (float)fileHeader.m_svoBounds.getMin().y, (float)fileHeader.m_svoBounds.getMin().z };
		float svoBoundsMax[3] = { (float)fileHeader.m_svoBounds.getMax().x, (float)fileHeader.m_svoBounds.getMax().y, (float)fileHeader.m_svoBounds.getMax().z };
		Singleton<AgRenderResourceManager>::instance().m_bboxManager.
			setBoundingBox(m_svoBounds, svoBoundsMin, svoBoundsMax);
		
		mHasRGB = fileHeader.mHasRGB;
		mHasNormals = fileHeader.mHasNormals;
		mHasIntensity = fileHeader.mHasIntensity;

		// get scan id
		mId = RCString(fileHeader.mScanId).w_str();
		mId.resize(38);

		//Compatible with some old files
		if (fileHeader.mCoordinateSystem[0] >= 0)
		{
			//get the coordinate system
			std::wstring originalCoordinateSystem = String::RCStringUtils::string2WString(fileHeader.mCoordinateSystem);

			//remove null character and compatible with old files
			int nullCharSize = 0;
			int csSize = sizeof(fileHeader.mCoordinateSystem);
			for (int i = csSize - 1; i > -1; i--)
			{
				if (fileHeader.mCoordinateSystem[i] == 0)
				{
					nullCharSize++;
				}
				else
				{
					break;
				}
			}
			originalCoordinateSystem.resize(csSize - nullCharSize);
			originalCoordinateSystem = String::RCStringUtils::trim(originalCoordinateSystem);
			RCCode result = setOriginalCoordinateSystem(originalCoordinateSystem);
			if (result != rcOK) {
				setOriginalCoordinateSystem(L""); //if CS string is not recognized just default to empty string
			}
		}

		m_amountOfChunks = fileHeader.m_numFileChunks;

		m_chunkList.clear();

		//parse chunks
		for (int i = 0; i < m_amountOfChunks; i++)
			m_chunkList.push_back(fileHeader.m_fileChunkEntries[i]);

		//some check
		assert(int(m_chunkList.size()) >= 2);

		if (legacyFileFormat)
			return rcLegacyFileFormat;

		return rcOK;
	}

	void AgVoxelTreeRunTime::checkSegmentInfo()
	{
		if (mHasCheckedSegmentInfo)
			return;

		if (hasStructuredChunk(m_chunkList.begin(), m_chunkList.end()))
		{
			RCMemoryMapFile memmap(this->getFileName().c_str());
			if (memmap.createFileHandleOnlyRead())
			{
				mHasSegmentInfo = true;
			}
			else
			{
				assert("Invalid File Handle!");
				mHasSegmentInfo = false;
			}
			memmap.closeFileHandle();
		}
		else
		{
			mHasSegmentInfo = false;
		}

		mHasCheckedSegmentInfo = true;
	}

	bool AgVoxelTreeRunTime::hasSegmentInfo() const
	{
		if (mHasCheckedSegmentInfo)
			return mHasSegmentInfo;
		else
			return false;
	}

	RCCode AgVoxelTreeRunTime::loadFromMedium(const std::wstring& projectFile, const std::wstring& fileName, bool isLightWeight)
	{
		bool legacyFileFormat = false;
		RCCode rcCode = rcFailed;
		if (Filesystem::exists(fileName))
		{
			// read file header
			rcCode = readFileHeader(fileName);
			{
				if (!projectFile.empty())
				{
					setProjectDirectory(projectFile);
					mRelativeFileName = Filesystem::makeRelative(projectFile, fileName);
				}

				if (rcCode != rcOK)
				{
					if (rcCode == rcLegacyFileFormat)
						legacyFileFormat = true;
					else
						return rcCode;
				}
			}

			//parse mandatory octree data   
			if (!parseVoxelData(fileName))
				return rcFileCorrupt;

			//build complete transform here, because other chunks might rely on it
			//buildTransform();

			//parse meta data
			parseOtherData(fileName);

			//const std::wstring mirrorBallTexFile = Filesystem::replaceExtension(fileName, MIRRORBALL_TEXTURE_EXTENSION);
			//setMirrorBallTextureFilePath(mirrorBallTexFile);

			//if not light weight project load other data
			if (!isLightWeight)
			{
				try
				{
					checkSegmentInfo();
					//parse(optional) segment data
					if (mHasSegmentInfo)
						parseSegmentData(fileName);
				}
				catch (std::bad_alloc)
				{
					return rcFailed;
				}
			}

			if (hasTimeStamp())
			{
				parseTimeStampData(fileName);
			}

			//update bounding box
			updateAll();
		}

		return rcCode;
	}

	bool AgVoxelTreeRunTime::parseOtherData(const std::wstring& fileName)
	{
		OctreeFileChunk othersChunk;
		if (getFileChunk(ENUM_FILE_CHUNK_ID_OTHERS, othersChunk))
		{
			RCMemoryMapFile memmap(fileName.c_str());
			if (memmap.createFileHandleOnlyRead())
			{
				int numInfo = 0;
				memmap.setFilePointer(othersChunk.m_fileOffsetStart);
				memmap.readFile(&numInfo, sizeof(int));

				std::int64_t othersOffset = othersChunk.m_fileOffsetStart + sizeof(int);
				int count = 0;
				int infoId = 0;
				int infoSize = 0;
				while (count < numInfo)
				{
					// get info id & info size
					infoId = memmap.readInt(othersOffset, othersOffset);
					infoSize = memmap.readInt(othersOffset, othersOffset);
					if (infoId == INFO_ID_NORMALIZE_INTENSITY)
					{
						mNormalizeIntensity = memmap.readBool(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_MAX_INTENSITY)
					{
						mMaxIntensity = memmap.readFloat(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_MIN_INTENSITY)
					{
						mMinIntensity = memmap.readFloat(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_RANGE_IMAGE_WIDTH)
					{
						mRangeImageWidth = memmap.readInt(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_RANGE_IMAGE_HEIGHT)
					{
						mRangeImageHeight = memmap.readInt(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_IS_LIDAR_DATA)
					{
						mIsLidarData = memmap.readBool(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_LIDAR_CLASSIFICATION)
					{
						for (int k = 0; k < 256; k++)
						{
							mLidarClassificationData[k] = memmap.readBool(othersOffset, othersOffset);
						}
					}
					else if (infoId == INFO_ID_RANGE_IMAGE_HOR_BEGIN_ANGLE)
					{
						mRangeImageHorizontalStartAngle = memmap.readDouble(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_RANGE_IMAGE_HOR_END_ANGLE)
					{
						mRangeImageHorizontalEndAngle = memmap.readDouble(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_RANGE_IMAGE_VER_BEGIN_ANGLE)
					{
						mRangeImageVerticalStartAngle = memmap.readDouble(othersOffset, othersOffset);
					}
					else if (infoId == INFO_ID_RANGE_IMAGE_VER_END_ANGLE)
					{
						mRangeImageVerticalEndAngle = memmap.readDouble(othersOffset, othersOffset);
					}
					else
					{
						othersOffset += infoSize;
					}

					count++;
				}//while

				memmap.closeFileHandle();

				return true;
			}
		}
		return false;
	}

	bool AgVoxelTreeRunTime::parseVoxelData(const std::wstring& fileName)
	{
		OctreeFileChunk umbrellaChunk; //= m_chunkList[0];
		OctreeFileChunk nodeDataChunk;// = m_chunkList[1];

									  //mandatory file chunks
		if (!getFileChunk(ENUM_FILE_CHUNK_ID_UMBRELLA_OCTREE_DATA, umbrellaChunk))
			return false;
		if (!getFileChunk(ENUM_FILE_CHUNK_ID_VOXEL_DATA, nodeDataChunk))
			return false;

		RCMemoryMapFile memmap(fileName.c_str());
		if (memmap.createFileHandleOnlyRead())
		{
			std::uint64_t numLeafs = 0;
			memmap.setFilePointer(umbrellaChunk.m_fileOffsetStart);
			memmap.readFile(&numLeafs, sizeof(std::uint64_t));

			if (numLeafs > 0)
			{
#if defined(TARGET_OS_IPHONE) && defined(__clang__)
				// NB: All memory allocations on iOS are 16-byte aligned; OctreeLeafSaveData should
				//     require at most 8-byte alignment, so the over-aligned warning is incorrect.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wover-aligned"
#endif
				OctreeLeafSaveData* leafDataList = new OctreeLeafSaveData[int(numLeafs)];
#if defined(TARGET_OS_IPHONE) && defined(__clang__)
#pragma clang diagnostic pop
#endif

				//point to leaf information
				memmap.setFilePointer(umbrellaChunk.m_fileOffsetStart + sizeof(numLeafs));
				memmap.readFile(&leafDataList[0], sizeof(OctreeLeafSaveData) * int(numLeafs));

				//m_originalLeafNodeList.reserve((int)numLeafs);
				m_leafNodeList.reserve((int)numLeafs);

				std::uint64_t cumulativeOffset = 0;
				std::uint64_t cumulativeUmbrellaOffset = umbrellaChunk.m_fileOffsetStart + sizeof(std::uint64_t);

				//fill in voxel containers
				for (int i = 0; i < int(numLeafs); i++)
				{
					OctreeLeafSaveData curLeaf = leafDataList[i];
					AgVoxelContainer* runTimeLod = new AgVoxelContainer(this);

					float nodeBoundsMin[3] = { (float)curLeaf.m_nodeBounds.getMin().x, (float)curLeaf.m_nodeBounds.getMin().y, (float)curLeaf.m_nodeBounds.getMin().z };
					float nodeBoundsMax[3] = { (float)curLeaf.m_nodeBounds.getMax().x, (float)curLeaf.m_nodeBounds.getMax().y, (float)curLeaf.m_nodeBounds.getMax().z };
					Singleton<AgRenderResourceManager>::instance().m_bboxManager.
						setBoundingBox(runTimeLod->m_nodeBounds, nodeBoundsMin, nodeBoundsMax);
					float svoBoundsMin[3] = { (float)curLeaf.m_svoBounds.getMin().x, (float)curLeaf.m_svoBounds.getMin().y, (float)curLeaf.m_svoBounds.getMin().z };
					float svoBoundsMax[3] = { (float)curLeaf.m_svoBounds.getMax().x, (float)curLeaf.m_svoBounds.getMax().y, (float)curLeaf.m_svoBounds.getMax().z };
					Singleton<AgRenderResourceManager>::instance().m_bboxManager.
						setBoundingBox(runTimeLod->m_svoBounds, svoBoundsMin, svoBoundsMax);

					runTimeLod->m_maximumLOD = curLeaf.m_maxDepth;
					runTimeLod->m_amountOfPoints = curLeaf.m_amountOfPoints;

					int cumLeafPoints = 0;
					int cumNodePoints = 0;

					for (int j = 0; j < 32; j++)
					{
						//cumulative stuff
						cumLeafPoints += curLeaf.m_numLeafNodes[j];
						cumNodePoints += curLeaf.m_numLeafNodes[j] + curLeaf.m_numNonLeafNodes[j];

						runTimeLod->m_amountOfOctreeNodes[j] = curLeaf.m_numLeafNodes[j] + curLeaf.m_numNonLeafNodes[j];
						runTimeLod->m_amountOfLODPoints[j] = curLeaf.m_numNonLeafNodes[j] + cumLeafPoints;
					}

					//suppress all the nodes below maxDepth level
					//but don't change the offset information in rcs file
					//note: runTimeLod->m_amountofPoints cannot be changed, otherwise segmentinfo and time stamp fetch will get wrong
					for (int jLevel = runTimeLod->m_maximumLOD; jLevel < 32; ++jLevel)
					{
						runTimeLod->m_amountOfOctreeNodes[jLevel] = 0;
						runTimeLod->m_amountOfLODPoints[jLevel] = runTimeLod->m_amountOfLODPoints[runTimeLod->m_maximumLOD - 1];
					}

					//////
					int pointSizeInBytes = sizeof(AgVoxelLeafNode)     * cumLeafPoints;
					int nodeSizeInBytes = sizeof(AgVoxelInteriorNode) * cumNodePoints;

					runTimeLod->m_nodeOffsetStart = cumulativeOffset + nodeDataChunk.m_fileOffsetStart;
					runTimeLod->m_pointDataOffsetStart = cumulativeOffset + nodeSizeInBytes + nodeDataChunk.m_fileOffsetStart;
					runTimeLod->m_umbrellaNodeOffsetStart = cumulativeUmbrellaOffset;

					//update cumulative offset
					cumulativeOffset += (pointSizeInBytes + nodeSizeInBytes);
					cumulativeUmbrellaOffset += sizeof(OctreeLeafSaveData);

					//store index into original leaf node list          
					runTimeLod->m_originalIndex = i;

					//add to list
					m_leafNodeList.push_back(runTimeLod);
					//also add a copy to the unmodified leaf node list
					//m_originalLeafNodeList.push_back(runTimeLod);
				}
				//free
				DeleteArr(leafDataList);
			}

			//setNodeName(Filesystem::basename(fileName));
			memmap.closeFileHandle();

			return true;
		}

		return false;
	}

	bool AgVoxelTreeRunTime::parseUmbrellaOctreeData(const std::wstring& fileName)
	{
		OctreeFileChunk umbrellaOctreeChunk;
		if (getFileChunk(ENUM_FILE_CHUNK_ID_UMBRELLA_EXTENDED_DATA, umbrellaOctreeChunk))
		{
			RCMemoryMapFile memmap(fileName.c_str());
			int nodeData[32];

			if (memmap.createFileHandleOnlyRead())
			{
				memmap.setFilePointer(umbrellaOctreeChunk.m_fileOffsetStart);
				memmap.readFile(&nodeData, sizeof(int) * 32);
			}

			int totalNodesRead = 0;

			for (int i = 0; i < 32; i++)
			{
				int numNodes = nodeData[i];
				if (!numNodes)
					break;

#if defined(TARGET_OS_IPHONE) && defined(__clang__)
				// NB: All memory allocations on iOS are 16-byte aligned; OctreeLeafIntermediateNodeSaveData
				//     should require at most 8-byte alignment, so the over-aligned warning is incorrect.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wover-aligned"
#endif
				OctreeLeafIntermediateNodeSaveData* newData = new OctreeLeafIntermediateNodeSaveData[numNodes];
#if defined(TARGET_OS_IPHONE) && defined(__clang__)
#pragma clang diagnostic pop
#endif

				memmap.setFilePointer((umbrellaOctreeChunk.m_fileOffsetStart + sizeof(int) * 32) + (sizeof(OctreeLeafIntermediateNodeSaveData) * totalNodesRead));
				memmap.readFile(&newData[0], sizeof(OctreeLeafIntermediateNodeSaveData) * numNodes);
				for (int j = 0; j < numNodes; j++)
				{
					/*alDebugBounds bounds;
					bounds.m_bounds = newData[j].m_svoBounds.toBound3f();
					bounds.m_color  = RCVector4f( newData[j].m_rgba.x / 255.0f, newData[j].m_rgba.y / 255.0f, newData[j].m_rgba.z / 255.0f, 1.0f );
					getWorld()->addDebugBound( bounds );*/
					if (newData[j].m_indexIntoLeafArray != -1)
					{
						if (newData[j].m_indexIntoLeafArray >= (int)m_leafNodeList.size())
						{
							assert("invalid index");
						}
					}

				}
				totalNodesRead += numNodes;
				DeleteArr(newData);
			}

			memmap.closeFileHandle();

			return true;
		}
		return false;
	}

	bool AgVoxelTreeRunTime::parseSegmentData(const std::wstring& fileName)
	{
		OctreeFileChunk segmentChunk;
		if (getFileChunk(ENUM_FILE_CHUNK_ID_VOXEL_SEGMENT_DATA, segmentChunk))
		{
			RCMemoryMapFile memmap(fileName.c_str());
			if (memmap.createFileHandleOnlyRead())
			{
				memmap.setFilePointer(segmentChunk.m_fileOffsetStart);
				memmap.readFile(&mAmountOfSegments, sizeof(int));

				std::uint64_t segmentOffset = segmentChunk.m_fileOffsetStart + sizeof(int);
				for (size_t i = 0; i < m_leafNodeList.size(); i++)
				{
					AgVoxelContainer* runTimeLod = m_leafNodeList[i];
					runTimeLod->m_segmentOffsetStarts = segmentOffset;
					//increase offset
					segmentOffset += sizeof(std::uint16_t) * runTimeLod->m_amountOfPoints;
				}

				memmap.closeFileHandle();

				return true;
			}
			else
				return false;
		}
		return false;
	}

	bool AgVoxelTreeRunTime::parseTimeStampData(const std::wstring& fileName)
	{
		OctreeFileChunk timeStampChunk;
		if (getFileChunk(ENUM_FILE_CHUNK_ID_TIME_STAMP, timeStampChunk))
		{
			RCMemoryMapFile memmap(fileName.c_str());
			if (memmap.createFileHandleOnlyRead())
			{
				memmap.setFilePointer(timeStampChunk.m_fileOffsetStart);
				int totalIntermediateLeafNodes = 0;
				memmap.readFile(&totalIntermediateLeafNodes, sizeof(int));

				std::uint64_t timeStampOffset = timeStampChunk.m_fileOffsetStart + sizeof(int);
				for (size_t i = 0; i < m_leafNodeList.size(); i++)
				{
					AgVoxelContainer* runTimeLod = m_leafNodeList[i];
					runTimeLod->m_timeStampOffsetStarts = timeStampOffset;
					//increase offset
					timeStampOffset += sizeof(double) * runTimeLod->m_amountOfPoints;
				}

				return true;
			}
			else
				return false;
		}
		return false;
	}

	bool AgVoxelTreeRunTime::onLoad(const std::wstring& projectFile, const AgVoxelTreeNode& treeNode, bool isLightWeight)
	{
		mId = treeNode.id;
		m_useFileHeaderTransform = false;
		Singleton<AgRenderResourceManager>::instance().m_transforms.
			setTransform(m_global_transform_h, treeNode.translation, treeNode.rotation, treeNode.scale);

		m_nodeColor = RCVector4f(treeNode.color.x, treeNode.color.y, treeNode.color.z, 1.0f);
		m_isVisible = treeNode.visible;
		//setSelected(treeNode.selected);
		m_registered = treeNode.registered;
		//setNodeName(treeNode.nodeName);
		//setGroupName(treeNode.group);

		bool fileExists = false;

		std::wstring fileName = treeNode.path;
		std::wstring existFileName = L"";
		if (Filesystem::findFirstExisting(existFileName, fileName, mSearchPaths))
		{
			m_fileName = existFileName;

			fileExists = true;

			//update relative RCS file path
			mRelativeFileName = Filesystem::makeRelative(mProjectDirectory, m_fileName, false);
		}

		if (!fileExists)
		{
			mProjectDirectory = Filesystem::isDirectory(projectFile)
				? projectFile
				: Filesystem::parentPath(projectFile);
			mRelativeFileName = treeNode.relativePath;

			const std::wstring fullPath = Filesystem::canonical(mRelativeFileName, mProjectDirectory);
			if (Filesystem::exists(fullPath))
			{
				fileExists = true;

				m_fileName = fullPath;
			}
		}

		m_totalAmountOfPoints = treeNode.numPoints;
		mHasRGB = treeNode.hasRGB;
		mHasNormals = treeNode.hasNormals;
		mHasIntensity = treeNode.hasIntensity;
		mNormalizeIntensity = treeNode.normalizeIntensity;
		mMaxIntensity = treeNode.maxIntensity;
		mMinIntensity = treeNode.minIntensity;
		mRangeImageWidth = treeNode.rangeImageWidth;
		mRangeImageHeight = treeNode.rangeImageHeight;
		mIsLidarData = treeNode.isLidarData;
		if (mIsLidarData)
		{
			for (size_t i = 0; i < treeNode.lidarClassifications.size(); i++)
			{
				if (i > 0 && i < 256)
					mLidarClassificationData[(int)i] = true;
			}
		}

		if (!fileExists)
		{
			return false;
		}
		else
		{
			// if the file exists, overwrite the variables with file content
			RCCode rcCode = loadFromMedium(projectFile, m_fileName, isLightWeight);
			if (rcCode != rcOK && rcCode != rcLegacyFileFormat)
				return false;

			// check the file id 
			if (mId != treeNode.id)
			{
				//This can happen an RCS created in another project is attached to a new project
				//for legacy reasons, do nothing.  This prefers the RCS id to the id in the RCP
				//once the projet is saved, these should be in sync again
			}
		}

		//buildTransform();

		//setNodeName(Filesystem::basename(m_fileName));

		return true;
	}
}