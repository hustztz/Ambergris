#pragma once

#include <string>

#include <common/RCBox.h>
#include <common/RCMath.h>
#include <common/RCScanProperties.h>

#ifndef ALIGNAS
#ifndef _MSC_VER
#define ALIGNAS(x) alignas(x)
#else
#define ALIGNAS(x)
#endif
#endif

namespace ambergris {
	namespace RealityComputing {
		namespace Common {
			class RCMemoryMapFile;
		}
	}
}

namespace ambergris {

		//REP 1.0
#define             OCTREE_VERSION_NUMBER               4
		//#define               OCTREE_RESERVED_BYTES               1024*128
#define             OCTREE_MAX_LEAF_POINTS              1000000

		//old way to encode octree octants
		const int           TOP_LEFT_FRONT = 0;
		const int           TOP_LEFT_BACK = 1;
		const int           TOP_RIGHT_FRONT = 2;
		const int           TOP_RIGHT_BACK = 3;
		const int           BOTTOM_LEFT_FRONT = 4;
		const int           BOTTOM_LEFT_BACK = 5;
		const int           BOTTOM_RIGHT_FRONT = 6;
		const int           BOTTOM_RIGHT_BACK = 7;

		//REP 2.0
		//version 3 add total amount of leaf points,
		//version 4 (optionally)compresses slices & add compressed image format
#define             SVO_VERSION_NUMBER                  4
#define             SVO_IS_LEAF                         (1<<8)
#define             SVO_VISITED                         (1<<9)   //Only relevant when building SVO
#define             SVO_IS_NOISE                        (1<<9)
#define             MAX_NODES_IN_SLICE                  65535    //sizeof std::uint16_t
#define             MAX_SLICES_IN_TREE                  16777214 //2^24 -2


#define             OCTREE_RESERVED_BYTES               512*1024 //these will be stored in the svo file

#define             U8_TO_FLOAT                         1.0/255.0
#define             U16_TO_FLOAT                        1.0/65535.0

		//FLAGS
#define             OCTREE_FLAGS_HAS_NORMALS            0x02
#define             OCTREE_FLAGS_HAS_ORIGINAL_POINTS    0x04    //if set octree has original points stored in the compressed image
#define             OCTREE_FLAGS_COMPRESSED_V2          0x08    //like v1. but with slices re-arranged to speed up streaming
#define             OCTREE_FLAGS_NOISE_FILTER_APPLIED   0x10
#define             OCTREE_FLAGS_LEAFS_SEGMENTED        0x20    //leaf nodes are segmented
#define             OCTREE_FLAGS_COMPRESSED_ZLIB        0x40

#define             OCTREE_USE_RELATIVE_OFFSET          1       //if '1' uses relative offset encoded as 36 bits per coord

#define             OCTREE_INDEX_BUFFER_WIDTH           256
#define             OCTREE_INDEX_BUFFER_HEIGHT          256

		//RAY CASTER
#define             RAY_CAST_IGNORE_NODE                (1<<31) //completely ignore this node
#define             RAY_CAST_LOAD_CHILDREN_OF_NODE      (1<<30) //tell the memory manager to load children from disk
#define             RAY_CAST_REMOVE_CHILDREN_OF_NODE    (1<<29) //tell the memory manager to remove children
#define             RAY_CAST_ALL_FLAGS                  RAY_CAST_LOAD_CHILDREN_OF_NODE|RAY_CAST_REMOVE_CHILDREN_OF_NODE

		//STREAMING/RENDERING
#define             VOXEL_MAX_LOADED_DEPTH              0x1F
#define             VOXEL_MAX_DRAW_DEPTH                0x1F
#define             VOXEL_CURRENTLY_ACCESSED            (1<<0)  //voxel is accessed in streaming thread so ignore
#define             VOXEL_MODIFIED_FROM_SELECTION       (1<<12) //voxel is modified from a point-cloud selection
#define             VOXEL_ALREADY_PROCESSED             (1<<14) //voxel is already added to visibiblity list
#define             VOXEL_ALL_CHILDS_LOADED             (1<<15)

#define             MAX_POINTCLOUDS_PER_PROJECT         1024
#define             OCTREE_NODE_INDEXING_VALUE          100     //conversion needed for gpu code to cpu code

#define             USE_OLD_STREAM_RENDER_METHOD        0

#define                 MAX_POINTS_PER_OCTREE_LEAFNODE          (  512 * 1024 - 1 )
#define                 MAX_INTERMEDIATE_TREE_DEPTH             ( 6 )
#define                 MIN_SVO_LEAF_POINTS                     1
#define                 MAJOR_VERSION                           3
#define                 MINOR_VERSION                           2

#define                 MAX_SIZE_TERRESTRIAL_UMBRELLA_LEAFNODE  2.5     //we can use 12 bytes per point
#define                 MAX_SIZE_LIDAR_UMBRELLA_LEAFNODE        262.0   //lidar/aerial data

		//future expansion of file format
#define                 MAX_FILE_CHUNK_ENTRIES                  4096

		//#define                   MAX_TEMP_POINTS_IN_FILE                 2000000

#define                 CHUNK_START_OFFSET                      297

		//Side that divide a voxel in one of the  possible halfspaces
#define X_HALFSPACE                                     0x00
#define Y_HALFSPACE                                     0x01
#define Z_HALFSPACE                                     0x02

		//voxel octants
#define LEFT_FRONT_BOTTOM                               ( 0 << X_HALFSPACE ) | ( 0 << Y_HALFSPACE  ) | ( 0 << Z_HALFSPACE )  //( 0, 0, 0 ) 0 -x, -y, -z
#define RIGHT_FRONT_BOTTOM                              ( 1 << X_HALFSPACE ) | ( 0 << Y_HALFSPACE  ) | ( 0 << Z_HALFSPACE )  //( 1, 0, 0 ) 1 +x, -y, -z
#define LEFT_BACK_BOTTOM                                ( 0 << X_HALFSPACE ) | ( 1 << Y_HALFSPACE  ) | ( 0 << Z_HALFSPACE )  //( 0, 1, 0 ) 2 -x, +y, -z
#define RIGHT_BACK_BOTTOM                               ( 1 << X_HALFSPACE ) | ( 1 << Y_HALFSPACE  ) | ( 0 << Z_HALFSPACE )  //( 1, 1, 0 ) 3 +x, +y, -z
#define LEFT_FRONT_TOP                                  ( 0 << X_HALFSPACE ) | ( 0 << Y_HALFSPACE  ) | ( 1 << Z_HALFSPACE )  //( 0, 0, 1 ) 4 -x, -y, +z
#define RIGHT_FRONT_TOP                                 ( 1 << X_HALFSPACE ) | ( 0 << Y_HALFSPACE  ) | ( 1 << Z_HALFSPACE )  //( 1, 0, 1 ) 5 +x, -y, +z
#define LEFT_BACK_TOP                                   ( 0 << X_HALFSPACE ) | ( 1 << Y_HALFSPACE  ) | ( 1 << Z_HALFSPACE )  //( 0, 1, 1 ) 6 -x, +y, +z
#define RIGHT_BACK_TOP                                  ( 1 << X_HALFSPACE ) | ( 1 << Y_HALFSPACE  ) | ( 1 << Z_HALFSPACE )  //( 1, 1, 1 ) 7 +x, +y, +z

		//Compresses a coordinate into it's polar coords, further compression can be achieved, by also encoding the qua- octants
		//of a polar coordinate.
#define                                 PI_NEW                  RealityComputing::Common::Math::Constants::pi //AL_PI didn't give us enough precision
#define                                 PI_NEWDIVIDEDBY1        ( 1.0 / PI_NEW ) // 1.0 / PI
#define                                 PI2_NEW                 ( 2.0 * PI_NEW ) // 2.0 * PI
#define                                 PI2_NEWDIVIDEDBY1       ( PI_NEWDIVIDEDBY1 * 0.5 ) // 2.0 * PI
		//floating point version of PI related constant
#define                                 FPI_NEW                 3.1415926535897932384626433832795f //AL_PI didn't give us enough precision
#define                                 FPI_NEWDIVIDEDBY1       ( 1.0f / FPI_NEW ) // 1.0 / PI
#define                                 FPI2_NEW                ( 2.0f * FPI_NEW ) // 2.0 * PI
#define                                 FPI2_NEWDIVIDEDBY1      ( FPI_NEWDIVIDEDBY1 * 0.5f ) // 2.0 * PI
#define                                 UNITS_PER_XY_ROTATION   0x1FFFFLL       //~1 mm error @ 100 meters
#define                                 UNITS_PER_YZ_ROTATION   0x1FFFFLL       //~1 mm error @ 100 meters
#define                                 UNITS_PER_REFLECTANCE   0xFFLL          //[0...255]
#define                                 MAX_MILIMETERS          0xFFFFFLL       //~512 meter
#define                                 OFFSET_YZ_ROTATION      17              //yz rotation bits starts here
#define                                 OFFSET_SCANNER_DISTANCE 34              //distance bits starts here
#define                                 OFFSET_REFLECTANCE      54              //reflectance bits starts here
#define                                 OFFSET_SEGMENTID        14              //segment id starts here
#define                                 UNITS_PER_SEGMENTID     0xFFF           //
#define                                 ANGLE_XY_INCREMENT      ( ( PI_NEW * 2.0 )   / (double) UNITS_PER_XY_ROTATION )
#define                                 ANGLE_YZ_INCREMENT      ( PI_NEW / (double) UNITS_PER_YZ_ROTATION )
#define                                 BAD_POINT_BIT           62

#define MAX_TREE_DEPTH              23

		const std::uint64_t FILE_PROPERTY_SIZE = 4096;          // reserve 4KB for file properties
		const std::uint64_t VOXEL_TREE_INFO_SIZE = 100;         // reserver 100 for voxel tree information

																//approximation of projected voxel sizes
		static float VOXEL_SCALE_RATIO[] = {
			1.0f / 1,         1.0f / 2,         1.0f / 4,         1.0f / 8,
			1.0f / 16,        1.0f / 32,        1.0f / 64,        1.0f / 128,
			1.0f / 256,       1.0f / 512,       1.0f / 1024,      1.0f / 2048,
			1.0f / 4096,      1.0f / 8192,      1.0f / 16384,     1.0f / 32768,
			1.0f / 65536,     1.0f / 131072,    1.0f / 262144,    1.0f / 524288,
			1.0f / 1048576,   1.0f / 2097152,   1.0f / 4194304,   1.0f / 8388608
		};

		enum OCTREE_FLAGS
		{
			ENUM_OCTREE_FLAGS_HAS_COLOR = 0x01,     //has color
			ENUM_OCTREE_FLAGS_HAS_INTENSITY = 0x02,     //has reflectance
			ENUM_OCTREE_FLAGS_HAS_NORMALS = 0x04,     //has normals( qsplat )
			ENUM_OCTREE_FLAGS_HAS_CLASSIFICATION_LIDAR = 0x08,     //has lidar info( buildings )
			ENUM_OCTREE_FLAGS_HAS_CLASSIFICATION_PRIMITIVES = 0x10,     //has classification per point/voxel
			ENUM_OCTREE_FLAGS_HAS_RANGE_IMAGE = 0x20,     //range(360 pano ) image is stored
			ENUM_OCTREE_FLAGS_COMPRESSED_ZLIB = 0x40,     //zlib? this is a TODO!!!
			ENUM_OCTREE_FLAGS_USE_COMPRESSED_VOXEL_FORMAT = 0x80,     //if set can use compressed voxel format( alDrawVoxelV2 ), terrestrial scanner based
			ENUM_OCTREE_FLAGS_HAS_TIME_STAMP = 0x100     //has time stamp for las point
		};

		//TODO ask Hailong/Yan/Oytun for more info on this
		enum PRIMITIVE_CLASSIFICATION
		{
			ENUM_PRIMITIVE_CLASSIFICATION_NONE = 0,
			ENUM_PRIMITIVE_CLASSIFICATION_NOISE = 1,
			ENUM_PRIMITIVE_CLASSIFICATION_PLANAR = 2,
			ENUM_PRIMITIVE_CLASSIFICATION_CYLINDER = 3,
			ENUM_PRIMITIVE_CLASSIFICATION_SPHERICAL = 4,
			ENUM_PRIMITIVE_CLASSIFICATION_EDGE = 5             //We support edges?
		};

		enum VOXEL_FILE_CHUNK_IDS
		{
			ENUM_FILE_CHUNK_ID_VOXEL_DATA = 1,            //Voxel data
			ENUM_FILE_CHUNK_ID_UMBRELLA_OCTREE_DATA = 2,            //
			ENUM_FILE_CHUNK_ID_UMBRELLA_EXTENDED_DATA = 3,
			ENUM_FILE_CHUNK_ID_RANGE_IMAGE_DATA = 4,            //Range image data(png)
			ENUM_FILE_CHNUN_ID_PREVIEW_IMAGE_DATA = 5,            //Preview image data
			ENUM_FILE_CHUNK_ID_SEGMENT_INFO = 6,            //segment information from panoramic range image
			ENUM_FILE_CHUNK_ID_VOXEL_SEGMENT_DATA = 7,            //segment id's for voxel data
			ENUM_FILE_CHUNK_ID_SEGMENT_IMAGE_DATA = 8,            //segment range image(png)
			ENUM_FILE_CHUNK_ID_RGB_IMAGE_DATA = 9,            //rgb image data( jpg)
			ENUM_FILE_CHUNK_ID_E57_PROPERTIES_DATA = 10,           //e57 properties
			ENUM_FILE_CHUNK_ID_OTHERS = 11,           //store other information needed
			ENUM_FILE_CHUNK_ID_OCCLUSION_INFORMATION = 12,
			ENUM_FILE_CHUNK_ID_TIME_STAMP = 13            //time stamp information for las data
		};


		enum SLICE_COMPRESSION
		{
			COMPRESSION_NONE = 0,
			COMPRESSION_ZLIB
		};

		enum META_DATA_CHUNKID
		{
			METADATAID_FILEPROPERTY = 1000,        // get point cloud properties, such as sensor model, temperature, relativeHumidity and so on
			METADATAID_VOXEL_TREE_INFO = 1001,     // get voxel tree information, such as hasNormal, hasColor, hasIntensity
			METADATAID_PREVIEW_IMAGE = 1002,           // preview image saved in rcs
		};

		enum EXTENDED_INFORMATION
		{
			INFO_ID_NORMALIZE_INTENSITY = 100,
			INFO_ID_MAX_INTENSITY = 101,
			INFO_ID_MIN_INTENSITY = 102,

			INFO_ID_RANGE_IMAGE_WIDTH = 201,
			INFO_ID_RANGE_IMAGE_HEIGHT = 202,
			INFO_ID_RANGE_IMAGE_HOR_BEGIN_ANGLE = 203,
			INFO_ID_RANGE_IMAGE_HOR_END_ANGLE = 204,
			INFO_ID_RANGE_IMAGE_VER_BEGIN_ANGLE = 205,
			INFO_ID_RANGE_IMAGE_VER_END_ANGLE = 206,

			INFO_ID_IS_LIDAR_DATA = 301,
			INFO_ID_LIDAR_CLASSIFICATION = 302
		};


		//////////////////////////////////////////////////////////////////////////
		//alSvoSliceHeader: Gives us information over the stored slices
		//////////////////////////////////////////////////////////////////////////
		struct alSvoSliceHeader
		{
			alSvoSliceHeader();

			std::int64_t                m_offsetOnDisk;         //file offset on disk
			std::uint32_t               m_sliceSizeInBytes;     //size of this slice on disk(in bytes)
		};

		//////////////////////////////////////////////////////////////////////////
		/// \brief: MetadataInformation stores information of metadata
		//////////////////////////////////////////////////////////////////////////
		struct MetadataInformation
		{
			int         mChunkID;
			std::uint64_t     mFileOffset;
			std::uint64_t     mSizeInBytes;
		};

		struct OctreeFileChunk
		{
			std::uint32_t                                       m_chunkId;
			char                                                dummy[4];
			std::uint64_t                                       m_fileOffsetStart;
			std::uint64_t                                       m_sizeInBytes;

			OctreeFileChunk() : m_chunkId(0), dummy{ 0 }, m_fileOffsetStart(0), m_sizeInBytes(0) { }
		};
		static_assert(sizeof(OctreeFileChunk) == 24, "sizeof(OctreeFileChunk) is incorrect"); // 20 + 4 pad

	//////////////////////////////////////////////////////////////////////////
	//OctreeFileHeader:
	//////////////////////////////////////////////////////////////////////////
#pragma pack(push)
#pragma pack(1)
		struct OctreeFileHeader
		{
			OctreeFileHeader();

			char                                                m_magicWord[8];             //  8
			std::uint32_t                                       m_majorVersion;             //  12
			std::uint32_t                                       m_minorVersion;             //  16

																							//transforms
			RealityComputing::Common::RCVector3d                                  m_translation;              //  40
			RealityComputing::Common::RCVector3d                                  m_rotation;                 //  64
			RealityComputing::Common::RCVector3d                                  m_scale;                    //  88
			RealityComputing::Common::RCVector3d                                  m_scannerOrigin;            //  112

																														//Bounding Boxes
			RealityComputing::Common::RCBox                                   m_svoBounds;  //SVO bounds      160
			RealityComputing::Common::RCBox                                   m_scanBounds; //Scan bounds     208

			std::uint32_t                                       m_octreeFlags;              //  212
			RealityComputing::Common::RCScanProvider            m_pointCloudProvider;       //  216
			std::uint64_t                                       m_totalAmountOfPoints;      //  224

			bool                                                mHasRGB;                    //  225
			bool                                                mHasNormals;                //  226
			bool                                                mHasIntensity;              //  227
			char                                                mScanId[38];                //  265, guid for scan
			char                                                mCoordinateSystem[32];      //  297, coordinate system name, reserved

																							//file chunks actually in this file
			std::uint32_t                                       m_numFileChunks;            //  301
																							//amount of different file chunk entries
			OctreeFileChunk                                     m_fileChunkEntries[MAX_FILE_CHUNK_ENTRIES];

		};
#pragma pack(pop)
		static_assert(alignof(OctreeFileHeader) == 1, "alignof(OctreeFileHeader) is incorrect");
		static_assert(sizeof(OctreeFileHeader) == 98605, "sizeof(OctreeFileHeader) is incorrect");

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable:4251)
#endif

		struct ScanInfo
		{
			std::wstring                    mScanName;
			std::wstring                    mScanGuid;
			std::wstring                    mScanDescription;
			std::wstring                    mSensorSerial;
			std::wstring                    mSensorVendor;
			std::wstring                    mSensorModel;
			std::wstring                    mSensorSoftware;
			std::wstring                    mSensorFirmware;
			std::wstring                    mSensorHardware;
			double                          mAcquisitionStart;
			double                          mAcquisitionEnd;
			double                          mTemperature;
			double                          mHumidity;
			double                          mAirPressure;

			~ScanInfo();
		};

		//////////////////////////////////////////////////////////////////////////
		// \brief: additional info stored in the RCS
		//////////////////////////////////////////////////////////////////////////
		struct OtherScanInfoStruct
		{

			OtherScanInfoStruct() :
				mIsLidar(true),
				mNormalizeIntensity(true),
				mIntensityMaxValue(0),
				mIntensityMinValue(0),
				mRangeImageWidth(0),
				mRangeImageHeight(0),
				mRangeImageAzStart(0),
				mRangeImageAzEnd(0),
				mRangeImageElStart(0),
				mRangeImageElEnd(0)
			{
				for (int i = 0; i < 256; ++i)
					mClassificationFlags[i] = false;
			}

			bool mIsLidar;
			bool mNormalizeIntensity;
			float mIntensityMaxValue;
			float mIntensityMinValue;
			int mRangeImageWidth;
			int mRangeImageHeight;
			double mRangeImageAzStart;
			double mRangeImageAzEnd;
			double mRangeImageElStart;
			double mRangeImageElEnd;
			bool mClassificationFlags[256];
		};

#if defined(_MSC_VER)
#pragma warning(pop)
#endif


		//////////////////////////////////////////////////////////////////////////
		//alSVOStorageNode:
		//////////////////////////////////////////////////////////////////////////
#pragma pack(push)
#pragma pack(1)
		class alSVOStorageNode
		{
		public:

			alSVOStorageNode();

			alSVOStorageNode(const alSVOStorageNode& other);

			std::uint32_t           getSliceId() const;
			std::uint16_t       getOffsetInSlice() const;

			void            setSliceId(int val);
			void            setOffsetInSlice(std::uint16_t val);

			void            enable();

			std::uint16_t       getNormal();
			void            setNormal(int normal);

			bool            isValid();

			bool            hasChild(int childNum) const;

			int             getNumChilds();

			//For Leaf Nodes we can use these extra data
			std::uint8_t*       getSliceIdAsPointerUnSafe();

			//Remove warning C4366
			//The pointer is aligned by default, but this data structure is unaligned because of packing,
			//so got warning C4366 on x64 platform when the member's address is assigned to an aligned pointer.
			//We can change the alignment of the structure to resolve this, but this will increase the memory usage.
			//Since no one use this function currently, we can comment it temporarily.
			//If we have to use it, we should change the alignment of the structure.
			//std::uint16_t*        getSliceOffsetAsPointerUnsafe();


			int             m_bitSet;           //normals and child information
			std::uint8_t            m_rgba[4];          //rgb specular power

		private:

			//slice id of child
			std::uint8_t            m_sliceId[3];
			//offset of first child -->
			std::uint16_t       m_offsetInSlice;
		};
#pragma pack(pop)


		//////////////////////////////////////////////////////////////////////////
		//alRunTimeSlice
		//////////////////////////////////////////////////////////////////////////
		class alRunTimeSlice
		{
		public:
			/*

			*/
			alRunTimeSlice(int numNodes, const std::wstring& fileName, std::uint64_t fileOffset, int sliceId);

			alRunTimeSlice(const std::wstring& fileName, std::uint64_t fileOffset, int sizeInBytes, int sliceId, bool isCompressed);


			~alRunTimeSlice();

			void                            allocateNodes();

			alSVOStorageNode*               getNodeList();

			const alSVOStorageNode&         getNodeAt(int index) const;

			//modifiable
			alSVOStorageNode*               getNodeAtAsPointer(int index);

			std::int64_t                            getAccessTime() const;

			/*
			Unload a slice that is stored in memory
			*\return the amount of bytes freed with the unload
			*/
			int                             unload();

			bool                            load();

			/*
			Loads a slice from disk, but doesn't close the file handle afterwards, useful when performing a lot of
			loads from disk, caller is responsible for closing the filehandle after the function call
			*/
			bool                            load(RealityComputing::Common::RCMemoryMapFile* memmapPtr);

			bool                            getLoaded() const;
			void                            setLoaded(bool val);

			std::uint64_t                           getFileOffset() const;

			int                             getSliceId() const;


			const std::wstring&                 getFileName() const;

			//bool                          compressSlice( int compressionType = COMPRESSION_ZLIB );

			//bool                          decompressSlice( int compressionType = COMPRESSION_ZLIB );

			/*
			Retrieve the amount in bytes that this slice allocated
			*\return the amount of bytes allocated with this slice
			*/
			int                             getAllocatedSizeInBytes();

		private:

			alSVOStorageNode*               m_nodeList;
			int                             m_numNodes;
			int                             m_compressedByteSize;
			void*                           m_compressedDataPtr;

			bool                            m_isCompressed;
			bool                            m_isLoaded;

			int                             m_sliceState;
			int                             m_memoryAllocated;
			int                             m_sliceId;

			std::wstring*                   m_fileName;
			std::uint64_t                           m_fileOffset;

			int                             m_compressedDataSize;
		};

		//////////////////////////////////////////////////////////////////////////
		// \brief: Stores the spatial information of a umbrella leaf node
		//////////////////////////////////////////////////////////////////////////
		class AgVoxelInteriorNode
		{
		public:

			/*
			SVO Interior Node ( 32 bits total )
			[0.....18]          Index to first child, if this is a leaf points to leaf array
			[19....26]          Child Information
			[27....27]          Is Leaf
			[28....31]          Reserved
			*/
			AgVoxelInteriorNode();
			AgVoxelInteriorNode(const AgVoxelInteriorNode& cpy);
			~AgVoxelInteriorNode();

			//////////////////////////////////////////////////////////////////////////
			// \brief: Sets the index to the first child, or points to the leaf index
			// array if it is a leaf
			//////////////////////////////////////////////////////////////////////////
			void                    setIndexToChild(int childIndex);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Sets the child information( all 8 of them )
			//////////////////////////////////////////////////////////////////////////
			void                    setChildInfo(int val);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Sets whether of not this node is a leaf, thus its
			//         child offset pointing to the leaf index
			//////////////////////////////////////////////////////////////////////////
			void                    setLeafInfo(int val);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the index to the first child
			//////////////////////////////////////////////////////////////////////////
			int                     getChildIndex() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Return the bitfield info about this node's sibbling
			//////////////////////////////////////////////////////////////////////////
			int                     getChildInfo() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Return if this node is a leaf or not
			//////////////////////////////////////////////////////////////////////////
			int                     getLeafInfo();

			void                    testData();


		private:

			std::uint32_t                               m_data;

		};
		static_assert(alignof(AgVoxelInteriorNode) == 4, "alignof(AgVoxelInteriorNode) is incorrect");
		static_assert(sizeof(AgVoxelInteriorNode) == 4, "sizeof(AgVoxelInteriorNode) is incorrect");

		//////////////////////////////////////////////////////////////////////////
		// \brief: Stores the spatial information of a umbrella leaf node
		//////////////////////////////////////////////////////////////////////////
		class ALIGNAS(8) AgVoxelLeafNode
		{
		public:

			/*
			SVO Leaf Node ( 128 bits total )
			//ULong #1
			[0.....53]          XYZ Coordinates( 18 bits per axis )
			[54....61]          Classification LIDAR
			[62]                Not used
			[63]                Delete flag for persistent deletion

			//ULong #2
			[0....31]           RGBA
			[32...45]           Normal ( QSplat Paper )     --> http://graphics.stanford.edu/papers/qsplat/
			[46...49]           Classification(PRIMITIVES)  --> Cylinder, Plane, Sphere, Edge, Noise, Smooth Surface, Undefined
			[50...63]           Reserved                    --> segment id?
			*/


			AgVoxelLeafNode();
			AgVoxelLeafNode(const AgVoxelLeafNode& cpy);
			~AgVoxelLeafNode();

			//////////////////////////////////////////////////////////////////////////
			//\brief: Set the offset, in meters, from the lower left part of the bounding box
			//////////////////////////////////////////////////////////////////////////
			void                    setRawOffsetFromBoundingBox(const RealityComputing::Common::RCVector3f& offset);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets the lidar classification data
			//////////////////////////////////////////////////////////////////////////
			void                    setLidarClassification(std::uint8_t val);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets a bit on indicating the point is deleted
			//////////////////////////////////////////////////////////////////////////
			void                    setDeleteFlag(bool onOff);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets the rgba data
			//////////////////////////////////////////////////////////////////////////
			void                    setRGBA(const RealityComputing::Common::RCVector4ub& rgba);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets the normal data
			//////////////////////////////////////////////////////////////////////////
			void                    setNormal(std::uint16_t normal);

			//////////////////////////////////////////////////////////////////////////
			//\brief: Sets the classification data
			//////////////////////////////////////////////////////////////////////////
			void                    setPrimitiveClassification(std::uint8_t val);

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the offset from the lower left part of the bounding box
			//////////////////////////////////////////////////////////////////////////
			RealityComputing::Common::RCVector3f      getRawOffsetFromBoundingBox() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the classification data
			//////////////////////////////////////////////////////////////////////////
			std::uint8_t            getLidarClassification() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the delete flag
			//////////////////////////////////////////////////////////////////////////
			bool            getDeleteFlag() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the RGBA data
			//////////////////////////////////////////////////////////////////////////
			RealityComputing::Common::RCVector4ub     getRGBA() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns the index into the normal lookup table(alMath.cpp)
			//////////////////////////////////////////////////////////////////////////
			std::uint16_t       getNormal() const;

			//////////////////////////////////////////////////////////////////////////
			// \brief: Returns identified primitives (by K2)
			//////////////////////////////////////////////////////////////////////////
			std::uint8_t            getPrimitiveClassification() const;

			void                    testData();

		private:
			std::uint64_t                                       m_data[2];
		};
		static_assert(alignof(AgVoxelLeafNode) == 8, "alignof(AgVoxelLeafNode) is incorrect");
		static_assert(sizeof(AgVoxelLeafNode) == 16, "sizeof(AgVoxelLeafNode) is incorrect");

		RealityComputing::Common::RCBox boundsForId(const RealityComputing::Common::RCBox& bounds, int id);
} // ns
