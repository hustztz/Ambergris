#include "AgRealityComputingManager.h"
#include "AgProjectFileExtensions.h"

#include <utility/RCFilesystem.h>

using namespace ambergris;
using namespace ambergris::RealityComputing::Common;
using namespace ambergris::RealityComputing::Utility;

AgVoxelTreeRunTime::Handle AgRealityComputingManager::loadPointCloudFile(const std::wstring& filename, RCCode& returnCode, const PointCloudLoadOptions& opts /*= PointCloudLoadOptions()*/)
{
	if (!Filesystem::exists(filename))
	{
		return AgVoxelTreeRunTime::kInvalidHandle;
	}

	if (Filesystem::lowercaseExtension(filename) == SCAN_FILE_EXTENSION)
	{
		std::wstring errCode;
		std::shared_ptr<AgVoxelTreeRunTime> newTreeFormat(new AgVoxelTreeRunTime());
		newTreeFormat->setUseFileHeaderTransform(opts.useFileHeaderTransform);
		returnCode = newTreeFormat->loadFromMedium(filename, m_projectFile, m_pointCloudProject.getLightWeight());
		if (returnCode == rcOK || returnCode == rcLegacyFileFormat)
		{
			if (m_pointCloudProject.addScan(newTreeFormat))
			{
				//update bounding boxes
				m_worldBounds.updateBounds(newTreeFormat->getBoundingBox());
				updateTransformedWorldBounds(newTreeFormat);

				//getPointSelectionManager()->getPointClipManager()->applyClipNodeListEffect(newTreeFormat);

				updateFarPlane();

				// open file stream so that file can't be renamed or moved
				/*std::wstring fileId = newTreeFormat->getId();
				openFileStream(fileId, filename);*/

				return newTreeFormat->m_handle;
			}
		}
	}

	return AgVoxelTreeRunTime::kInvalidHandle;
}


bool AgRealityComputingManager::unloadPointCloudFile(const std::wstring& fileId)
{
	// traverse available trees and unload the given file
	for (uint16_t ii = 0; ii < m_pointCloudProject.getSize(); ii++)
	{
		const AgVoxelTreeRunTime* treeIte = m_pointCloudProject.get(ii);
		if(!treeIte || AgVoxelTreeRunTime::kInvalidHandle == treeIte->m_handle)
			continue;

		// get file id of VoxelTreeRunTime
		if (treeIte->getId() == fileId)
		{
			m_pointCloudProject.removeScan(fileId);
			// close the file stream
			//closeFileStream(fileId);
			return true;
		}
	}

	return false;
}

RCCode AgRealityComputingManager::loadProject(const AgProjectInfoMgr& infoMgr, bool autoSetGeoRef = false)
{
	std::wstring fileName = infoMgr.getProjectFileName();

	if (fileName.empty() || Filesystem::lowercaseExtension(fileName) != PROJECT_FILE_EXTENSION)
	{
		return rcFalseArgument;
	}

	if (!Filesystem::exists(fileName))
	{
		return rcFileNotExist;
	}

	//already loaded, filename is the same
	if (fileName == m_projectFile)
	{
		return rcOK;
	}

	//unload current project
	unLoadProject();

	UnitType unitType;
	int unitPrecision;
	bool isUnitAdapted;
	infoMgr.getUnitInfo(unitType, unitPrecision, isUnitAdapted);
	RCUnitService::setUnitType(unitType);
	RCUnitService::setUnitPrecision(unitPrecision);
	RCUnitService::setUnitAdapted(isUnitAdapted);

	RCCode returnCode = loadWorldFromProjectInfoMgr(infoMgr, autoSetGeoRef);
	if (returnCode == rcOK || returnCode == rcLegacyFileFormat || returnCode == rcFileNotExist)
	{
		m_projectFile = fileName;
		mProjectInfoMgr.reset(new RCProjectInfoMgr(this, L""));
		mProjectInfoMgr->CopyFrom(infoMgr);

		updateRamps();

		// add this after updateRamps as it will change elevation range start/end
		loadColorRampFromProjectInfoMgr(infoMgr);
		m_pointCloudProject.setIsProjectDirty(false);

		return returnCode;
	}

	return returnCode;
}