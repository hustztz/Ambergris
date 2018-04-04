#include "AgOctreeDefinitions.h"

using namespace ambergris;
using namespace ambergris::RealityComputing::Common;

OctreeFileHeader::OctreeFileHeader()
	: m_magicWord{ 0 }, m_majorVersion(MAJOR_VERSION), m_minorVersion(MINOR_VERSION),
	m_scale(RCVector3d(1.0, 1.0, 1.0)),
	m_octreeFlags(0), m_pointCloudProvider(RCScanProvider::PROVIDER_UNKNOWN),
	m_totalAmountOfPoints(0), mHasRGB(false), mHasNormals(false), mHasIntensity(false),
	mScanId{ 0 }, mCoordinateSystem{ 0 }, m_numFileChunks(0)
{
	m_magicWord[0] = 'A';
	m_magicWord[1] = 'D';
	m_magicWord[2] = 'O';
	m_magicWord[3] = 'C';
	m_magicWord[4] = 'T';
}