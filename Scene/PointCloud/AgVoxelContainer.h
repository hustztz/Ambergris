#pragma once

#include "AgVoxelLidarPoint.h"
#include "AgVoxelTerrestialPoint.h"
#include "AgOctreeDefinitions.h"
#include "AgEngineSpatialFilter.h"

#include <utility/RCMutex.h>

#include <vector>
#include <memory>

namespace ambergris {

	namespace RealityComputing {
		namespace Common {
			class RCMemoryMapFile;
		}
	}
	class AgVoxelTreeRunTime;
	class CropVolume;

	//////////////////////////////////////////////////////////////////////////
	//\brief:  VoxelContainer 'Container' for storing pointcloud/voxel cache
	//////////////////////////////////////////////////////////////////////////
	class AgVoxelContainer
	{
	public:
		enum ClipFlag
		{
			ALL_CLIPPED = 0,
			NON_CLIPPED,
			PARTIAL_CLIPPED
		};

		//////////////////////////////////////////////////////////////////////////
		// \brief: For each voxel container also keep track which are its neighbour
		//         containers across multiple scans & itself
		//////////////////////////////////////////////////////////////////////////
		struct NeighbourInformation
		{
			int                                     m_scanId;
			int                                     m_containerId;
			float                                   m_distanceTo;   //distance to source containers
		};

		//////////////////////////////////////////////////////////////////////////
		// \brief: When deleting an region we need to skip, subsequent selections
		//         that are already in the voxel container list,
		//////////////////////////////////////////////////////////////////////////
		struct RegionDelete
		{
			//alPointSelect*                  m_curSelection;         //current selection that is removed
			int                             m_selectionIndex;       //next valid index to where selections should continue to apply
		};
	public:

		AgVoxelContainer(AgVoxelTreeRunTime* parentTreePtr);

		//////////////////////////////////////////////////////////////////////////
		// \brief: load a LIDAR LOD from disk this is a
		//////////////////////////////////////////////////////////////////////////
		void                                    loadLidarLODInternal(int lodLevel, RealityComputing::Common::RCMemoryMapFile* memMapFile = NULL, bool lock = false);

		//////////////////////////////////////////////////////////////////////////
		// \brief: load a terrestrial LOD from disk,and update the internal cache
		//////////////////////////////////////////////////////////////////////////
		void                                    loadTerrestialLODInternal(int newLOD, RealityComputing::Common::RCMemoryMapFile* memMapFile = NULL, bool lock = false);

		//////////////////////////////////////////////////////////////////////////
		// \brief: load a LIDAR LOD from disk, but without selection performed
		//////////////////////////////////////////////////////////////////////////
		void                                    loadLidarLODInternalNoSelection(int lodLevel, RealityComputing::Common::RCMemoryMapFile* memMapFile = NULL);

		//////////////////////////////////////////////////////////////////////////
		// \brief: load a terrestrial LOD from disk,and update the internal cache, but without selection performed
		//////////////////////////////////////////////////////////////////////////
		void                                    loadTerrestialLODInternalNoSelection(int newLOD, RealityComputing::Common::RCMemoryMapFile* memMapFile = NULL);

		//////////////////////////////////////////////////////////////////////////
		// \brief: load a LOD Point from disk, and return the pointList,
		//         it is a const function, which will not change either
		//          m_terrestialPointList or m_lidarPointList
		//TODO: cache the points...
		//////////////////////////////////////////////////////////////////////////
		std::vector<AgVoxelLeafNode>              loadLODPointToPointList(int newLOD, RealityComputing::Common::RCMemoryMapFile* memMapFile = NULL)  const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: load specified number of points
		//
		//////////////////////////////////////////////////////////////////////////
		void                                    loadRawPoints(int pointNumber, RealityComputing::Common::RCMemoryMapFile* memMapFile, std::vector<AgVoxelLeafNode>& out) const;

		//////////////////////////////////////////////////////////////////////////
		// \brief: load points with caller managing disk access
		//////////////////////////////////////////////////////////////////////////
		void                                    getLODInternalLoadInfo(int newLOD, std::uint64_t& fileOffset, int& bytesToRead) const;
		void                                    loadLODInternalFromRawData(int newLOD, void* data, int numBytes);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Write the currently loaded data to disk
		//////////////////////////////////////////////////////////////////////////
		bool                                    writeToDisk();

		//////////////////////////////////////////////////////////////////////////
		// \brief: update clip effect when refine points
		//
		//////////////////////////////////////////////////////////////////////////
		void                                    updateClipEffectWhenRefinePoints(size_t pointStartIndex);

		void                                    updateLayerEffectWhenRefinePoints(size_t pointStartIndex);

		//////////////////////////////////////////////////////////////////////////
		// \brief: return the amount of residual points after cropping
		//////////////////////////////////////////////////////////////////////////
		int                                     getResidualPointsNum();

		//////////////////////////////////////////////////////////////////////////
		// \brief: Clear all viewport ID's
		//////////////////////////////////////////////////////////////////////////
		void                                    clearAllViewId();

		//////////////////////////////////////////////////////////////////////////
		// \brief: Clears current view id(bit)
		//////////////////////////////////////////////////////////////////////////
		void                                    clearView(int id);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Sets current view id( bit)
		//////////////////////////////////////////////////////////////////////////
		void                                    setView(int id);

		//////////////////////////////////////////////////////////////////////////
		// \brief: Returns if the current view bit has been set
		//////////////////////////////////////////////////////////////////////////
		int                                     hasView(int id) const;

		//////////////////////////////////////////////////////////////////////////
		// \brief:  get all visible(visible layer) leaf nodes' indices, but limit box
		//          is ignored, so it's necessary to use limit box to check the visibility
		//          on screen
		//////////////////////////////////////////////////////////////////////////
		std::vector<int>                        getVisibleNodeIndices();

		void                                    setTransformedCenter(const RealityComputing::Common::RCVector3d& center);
		RealityComputing::Common::RCVector3d                      getTransformedCenter() const;

		void                                    setTransformedSVOBound(const RealityComputing::Common::RCBox& rhs);
		RealityComputing::Common::RCBox                        getTransformedSVOBound() const;
		RealityComputing::Common::RCBox                        getMentorTransformedSVOBound() const;
		RealityComputing::Common::RCBox                        getSVOBound() const;
		void                                    setTransformedNodeRadius(double rhs);
		double                                  getTransformedNodeRadius() const;

		bool                                    isComplete(int LOD) const;

		double                                  getFinestBoundLength();

		void updatePersistentDeleteFlags(RealityComputing::Common::RCMemoryMapFile* memMapFile);
		void clearPersistentDeleteFlags(RealityComputing::Common::RCMemoryMapFile* memMapFile);
		/*bool generateVoxelTempFiles(RealityComputing::Common::RCMemoryMapFile* memMapFile, const std::wstring& tempFile, const std::wstring& tempFolder,
			RealityComputing::Import::VoxelContainerStatus& voxelStatus,
			RealityComputing::Import::OctreeIntermediateNode<RealityComputing::Import::BasicOctreeImportPoint>& intermediateNode);*/

		//if this container has an occluding polygon, otherwise normal frustum culling should be applied
		bool                                    m_hasOccluder;



		//TODO make private
		AgVoxelTreeRunTime*                       m_parentTreePtr;
		int                                     m_currentLODLoaded;             //LOD level loaded in cache
		int                                     m_currentDrawLOD;               //LOD currently used for drawing
		int                                     m_maximumLOD;                   //maximum LOD stored in file
		int                                     m_amountOfPoints;               //total amount of points

		uint64_t                           m_nodeOffsetStart;              //offset to svo data
		uint64_t                           m_pointDataOffsetStart;         //offset to point data
		uint64_t                           m_umbrellaNodeOffsetStart;       //offset to voxel meta data

		uint64_t                           m_segmentOffsetStarts;          //offset to segments( this is in a different file chunk );

		uint64_t                           m_timeStampOffsetStarts;        //offset to time stamp data( this is in a different file chunk );

		int                                     m_amountOfOctreeNodes[32];      //amount of nodes leaf & non leaf per level
		int                                     m_amountOfLODPoints[32];        //amount of draw points per level ( approximately )

		int                                     m_originalIndex;                //original index into VoxelTreeRunTime::m_originalScanContainerList;
		int                                     m_modfifiedIndex;               //index after modifying

		uint32_t                           m_viewId;
		int                                     m_visibleListId;
		std::vector<NeighbourInformation>       m_neighbourList;                //neighbor list( intersecting voxel containers )


		RealityComputing::Common::RCBox                       m_nodeBounds,                   //node bounds
			m_svoBounds;                    //voxel bounds
											//TODO: we shall check if we are allowed to change the stack sequence of class members
											//      and stack public/private member together.
	private:
		RealityComputing::Common::RCBox                       m_transformedSVOBounds;         //transformed bounds
	public:
		double                          m_lastTimeModified;             //last time this node was modified/seen
																		//TODO: we shall check if we are allowed to change the stack sequence of class members
																		//      and stack public/private member together.
	private:
		double                                  m_nodeRadius;                   //radius of this container in meters
	public:
		//TODO move this to interfaces?
		std::vector<AgVoxelTerrestialPoint>       m_terrestialPointList;
		std::vector<std::uint16_t>              m_terrestialSegIdList;

		std::vector<AgVoxelLidarPoint>            m_lidarPointList;
		std::vector<double>                     m_timeStampList;

		int                                     m_numOldPoints;

		std::vector<int>                        m_relatedLayers;

		std::vector<RegionDelete>               m_selectionUndoRedo;            //needed for deleting of region

		CropVolume*                             m_cropVolumes;                  //store crop volumes' information

		std::unique_ptr<RealityComputing::Utility::Threading::RCMutex>          m_loadMutexPtr, m_diskMutexPtr;
		RealityComputing::Common::RCVector3d                      m_transformedCenter;            //center of transformed bounds

	public:
		// \brief:  clip status function
		bool                                    isClipFlag(ClipFlag rhs) const;
		ClipFlag                                getClipFlag() const;
		ClipFlag                                getInternalClipFlag() const;
		bool                                    isInternalClipFlag(ClipFlag rhs) const;
		void                                    setExternalClipFlag(ClipFlag rhs);
		void                                    setInternalClipFlag(ClipFlag rhs);
		void                                    updateClipFlag();

		AgEngineSpatialFilter::FilterResult       getInternalClipEngineFilterResult() const;

		int                                     clipIndex() const;
		void                                    setClipIndex(int nClipIndex);

		// \brief: set the region flag as invalid
		void                                    setRegionFlagInvalid();
		// \brief: set the region flag as valid
		void                                    setRegionFlagValid();
		// \brief: if a region flag is valid
		bool                                    isRegionFlagValid() const;
		// \brief: set the region index
		void                                    setRegionIndex(uint8_t index);
		// \brief: only valid when isRegionFlagValid returns true
		uint8_t                            getRegionIndex() const;

		bool                                    isInheritRegionFlagValid() const;
		uint8_t                            getInheritRegionIndex() const;

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

		inline void setHasPeristentDeletedPts(bool has) { mHasPersistentDeletedPts = has; }
		inline bool getHasPeristentDeletedPts() { return mHasPersistentDeletedPts; }

	private:
		// \brief: clip status
		ClipFlag                                m_clipFlag;
		ClipFlag                                mExternalClipFlag;
		ClipFlag                                mInternalClipFlag;

		// \brief: the index which indicates the point number of valid clip flag
		int                                     m_nClipIndex;
		// \brief: unsigned short m_regionFlag
		/*
		*[0] whether in_temp_selection_flag is valid
		*[1] whether the voxel is inside temp selection or
		*    outside temp selection if in_temp_selection is valid
		*[2] whether in_region_flag is valid
		*[3...9] the region index if in region flag is valid
		*/
		unsigned short                          m_regionFlag;

		bool mHasPersistentDeletedPts;
	};
}
