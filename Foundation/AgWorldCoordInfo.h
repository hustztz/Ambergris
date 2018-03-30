#pragma once

#include <common/RCVector.h>
#include <common/RCTransform.h>
#include <common/RCBox.h>
#include <common/RCOrientedBoundingBox.h>

namespace ambergris {

	//////////////////////////////////////////////////////////////////////////
	// \brief: coordinate converter for 4 coordinates: world -> Geo Reference -> UCS
	//   Terminology:
	//   1. World coordinate: world
	//   3. GeoGlobal: world -> UCS
	//   4. Global: world -> Geo Reference -> UCS
	//   Interfaces usage:
	//   (1.1) World -> GeoGlobal: display a point in UI
	//   (1.2) GeoGlobal -> World: convert UI value into world value
	//   (2.1) World -> Global: rendering point
	//   (2.2) Global -> World: convert rendering point into World
	//   (3.1) Global -> GeoGlobal:
	//   (3.2) GeoGlobal -> Global:
	//////////////////////////////////////////////////////////////////////////
	class AgWorldCoordInfo
	{
	public:
		AgWorldCoordInfo();
		~AgWorldCoordInfo();

		const RealityComputing::Common::RCVector3d&   getGeoReference() const;
		void                        setGeoReference(const RealityComputing::Common::RCVector3d& geoReference);

		const RealityComputing::Common::RCTransform&   getToGlobalFromWorld() const;
		void                        setToGlobalFromWorld(const RealityComputing::Common::RCTransform& toGlobalFromWorld);
		const RealityComputing::Common::RCTransform&   getInvToGlobalFromWorld() const;

		//\brief: World <-> GeoReferenceGlobal
		RealityComputing::Common::RCVector3d          worldToGeoGlobal(const RealityComputing::Common::RCVector3d& point) const;
		RealityComputing::Common::RCVector3f          worldToGeoGlobal(const RealityComputing::Common::RCVector3f& point) const;
		RealityComputing::Common::RCBox               worldToGeoGlobal(const RealityComputing::Common::RCBox& box) const;
		RealityComputing::Common::RCOrientedBoundingBox worldToGeoGlobal(const RealityComputing::Common::RCOrientedBoundingBox& box) const;
		RealityComputing::Common::RCTransform         worldToGeoGlobal(const RealityComputing::Common::RCTransform& transform) const;

		RealityComputing::Common::RCVector3d          geoGlobalToWorld(const RealityComputing::Common::RCVector3d& point) const;
		RealityComputing::Common::RCVector3f          geoGlobalToWorld(const RealityComputing::Common::RCVector3f& point) const;
		RealityComputing::Common::RCBox               geoGlobalToWorld(const RealityComputing::Common::RCBox& box) const;
		RealityComputing::Common::RCOrientedBoundingBox geoGlobalToWorld(const RealityComputing::Common::RCOrientedBoundingBox& box) const;
		RealityComputing::Common::RCTransform         geoGlobalToWorld(const RealityComputing::Common::RCTransform& transform) const;

		//\brief: World <-> Global
		RealityComputing::Common::RCVector3d          worldToGlobal(const RealityComputing::Common::RCVector3d& point) const;
		RealityComputing::Common::RCVector3f          worldToGlobal(const RealityComputing::Common::RCVector3f& point) const;
		RealityComputing::Common::RCBox               worldToGlobal(const RealityComputing::Common::RCBox& box) const;
		RealityComputing::Common::RCOrientedBoundingBox worldToGlobal(const RealityComputing::Common::RCOrientedBoundingBox& box) const;
		RealityComputing::Common::RCTransform         worldToGlobal(const RealityComputing::Common::RCTransform& transform) const;

		RealityComputing::Common::RCVector3d          globalToWorld(const RealityComputing::Common::RCVector3d& point) const;
		RealityComputing::Common::RCVector3f          globalToWorld(const RealityComputing::Common::RCVector3f& point) const;
		RealityComputing::Common::RCBox               globalToWorld(const RealityComputing::Common::RCBox& box) const;
		RealityComputing::Common::RCOrientedBoundingBox globalToWorld(const RealityComputing::Common::RCOrientedBoundingBox& box) const;
		RealityComputing::Common::RCTransform         globalToWorld(const RealityComputing::Common::RCTransform& transform) const;

		//\brief: Global <-> GeoReferenceGlobal
		RealityComputing::Common::RCVector3d          globalToGeoGlobal(const RealityComputing::Common::RCVector3d& point) const;
		RealityComputing::Common::RCBox               globalToGeoGlobal(const RealityComputing::Common::RCBox& box) const;
		RealityComputing::Common::RCOrientedBoundingBox globalToGeoGlobal(const RealityComputing::Common::RCOrientedBoundingBox& box) const;
		RealityComputing::Common::RCTransform         globalToGeoGlobal(const RealityComputing::Common::RCTransform& transform) const;

		RealityComputing::Common::RCVector3d          geoGlobalToGlobal(const RealityComputing::Common::RCVector3d& point) const;
		RealityComputing::Common::RCBox               geoGlobalToGlobal(const RealityComputing::Common::RCBox& box) const;
		RealityComputing::Common::RCOrientedBoundingBox geoGlobalToGlobal(const RealityComputing::Common::RCOrientedBoundingBox& box) const;
		RealityComputing::Common::RCTransform         geoGlobalToGlobal(const RealityComputing::Common::RCTransform& transform) const;

		//\brief: World Direction <-> Global Direction
		RealityComputing::Common::RCVector3d          worldDirection2GlobalDirection(const RealityComputing::Common::RCVector3d& dir) const;
		RealityComputing::Common::RCVector3d          globalDirection2worldDirection(const RealityComputing::Common::RCVector3d& dir) const;

	private:
		RealityComputing::Common::RCVector3d              mGeoReference;
		RealityComputing::Common::RCTransform              mToGlobalFromWorld;
		RealityComputing::Common::RCTransform              mToWorldFromGlobal;
	};
}