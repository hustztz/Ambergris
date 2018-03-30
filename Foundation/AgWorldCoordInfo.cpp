#include "AgWorldCoordInfo.h"

#include <common/RCBox.h>

using namespace ambergris;
using namespace ambergris::RealityComputing::Common;

///////////////////////////////////////////////////////////////////////////////
// \brief: constructor
///////////////////////////////////////////////////////////////////////////////
AgWorldCoordInfo::AgWorldCoordInfo()
{
	mGeoReference = RCVector3d();
	mToGlobalFromWorld = RCTransform();
	mToWorldFromGlobal = RCTransform();
}

///////////////////////////////////////////////////////////////////////////////
// \brief: destructor
///////////////////////////////////////////////////////////////////////////////
AgWorldCoordInfo::~AgWorldCoordInfo()
{
}

///////////////////////////////////////////////////////////////////////////////
// \brief: get Geo Reference
///////////////////////////////////////////////////////////////////////////////
const RCVector3d& AgWorldCoordInfo::getGeoReference() const
{
	return mGeoReference;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: set Geo Reference
///////////////////////////////////////////////////////////////////////////////
void AgWorldCoordInfo::setGeoReference(const RCVector3d& geoReference)
{
	mGeoReference = geoReference;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: get matrix ToGlobalFromWorld
///////////////////////////////////////////////////////////////////////////////
const RCTransform& AgWorldCoordInfo::getToGlobalFromWorld() const
{
	return mToGlobalFromWorld;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: set matrix ToGlobalFromWorld
///////////////////////////////////////////////////////////////////////////////
void AgWorldCoordInfo::setToGlobalFromWorld(const RCTransform& toGlobalFromWorld)
{
	mToGlobalFromWorld = toGlobalFromWorld;
	mToWorldFromGlobal = toGlobalFromWorld.getInverse(TransformType::Rigid);
}

///////////////////////////////////////////////////////////////////////////////
// \brief: get matrix InvToGlobalFromWorld
///////////////////////////////////////////////////////////////////////////////
const RCTransform& AgWorldCoordInfo::getInvToGlobalFromWorld() const
{
	return mToWorldFromGlobal;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: World -> GeoReferencedGlobal
///////////////////////////////////////////////////////////////////////////////
RCVector3d AgWorldCoordInfo::worldToGeoGlobal(const RCVector3d& point) const
{
	return mToGlobalFromWorld * point;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: World -> GeoReferencedGlobal
///////////////////////////////////////////////////////////////////////////////
RCVector3f AgWorldCoordInfo::worldToGeoGlobal(const RCVector3f& point) const
{
	return (mToGlobalFromWorld * point.convertTo<double>()).convertTo<float>();
}

///////////////////////////////////////////////////////////////////////////////
// \brief: World -> GeoReferencedGlobal
///////////////////////////////////////////////////////////////////////////////
RCBox AgWorldCoordInfo::worldToGeoGlobal(const RCBox& box) const
{
	return box.getTransformed(mToGlobalFromWorld);
}

RCOrientedBoundingBox AgWorldCoordInfo::worldToGeoGlobal(const RCOrientedBoundingBox& box) const
{
	auto t = mToGlobalFromWorld * box.getTransform();
	return RCOrientedBoundingBox(t, box.getSize());
}

RCTransform AgWorldCoordInfo::worldToGeoGlobal(const RCTransform& transform) const
{
	return mToGlobalFromWorld * transform;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: GeoReferencedGlobal -> World
///////////////////////////////////////////////////////////////////////////////
RCVector3d AgWorldCoordInfo::geoGlobalToWorld(const RCVector3d& point) const
{
	return mToWorldFromGlobal * point;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: GeoReferencedGlobal -> World
///////////////////////////////////////////////////////////////////////////////
RCVector3f AgWorldCoordInfo::geoGlobalToWorld(const RCVector3f& point) const
{
	return (mToWorldFromGlobal * point.convertTo<double>()).convertTo<float>();
}

///////////////////////////////////////////////////////////////////////////////
// \brief: GeoReferencedGlobal -> World
///////////////////////////////////////////////////////////////////////////////
RCBox AgWorldCoordInfo::geoGlobalToWorld(const RCBox& box) const
{
	return box.getTransformed(mToWorldFromGlobal);
}

RCOrientedBoundingBox AgWorldCoordInfo::geoGlobalToWorld(const RCOrientedBoundingBox& box) const
{
	auto t = mToWorldFromGlobal * box.getTransform();
	return RCOrientedBoundingBox(t, box.getSize());
}

RCTransform AgWorldCoordInfo::geoGlobalToWorld(const RCTransform& transform) const
{
	return mToWorldFromGlobal * transform;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: World -> Global
///////////////////////////////////////////////////////////////////////////////
RCVector3d AgWorldCoordInfo::worldToGlobal(const RCVector3d& point) const
{
	return mToGlobalFromWorld * (point - mGeoReference);
}

///////////////////////////////////////////////////////////////////////////////
// \brief: World -> Global
///////////////////////////////////////////////////////////////////////////////
RCVector3f AgWorldCoordInfo::worldToGlobal(const RCVector3f& point) const
{
	return (mToGlobalFromWorld * (point.convertTo<double>() - mGeoReference)).convertTo<float>();
}

///////////////////////////////////////////////////////////////////////////////
// \brief: World -> Global
///////////////////////////////////////////////////////////////////////////////
RCBox AgWorldCoordInfo::worldToGlobal(const RCBox& box) const
{
	RCBox globalBox(box.getMin() - mGeoReference, box.getMax() - mGeoReference);
	globalBox = globalBox.getTransformed(mToGlobalFromWorld);
	return globalBox;
}

RCOrientedBoundingBox AgWorldCoordInfo::worldToGlobal(const RCOrientedBoundingBox& box) const
{
	auto t = worldToGlobal(box.getTransform());
	return RCOrientedBoundingBox(t, box.getSize());
}

RCTransform AgWorldCoordInfo::worldToGlobal(const RCTransform& transform) const
{
	auto t = transform;
	t.translate(-mGeoReference);
	return (mToGlobalFromWorld * t);
}

///////////////////////////////////////////////////////////////////////////////
// \brief: Global -> World
///////////////////////////////////////////////////////////////////////////////
RCVector3d AgWorldCoordInfo::globalToWorld(const RCVector3d& point) const
{
	return (mToWorldFromGlobal * point) + mGeoReference;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: Global -> World
///////////////////////////////////////////////////////////////////////////////
RCVector3f AgWorldCoordInfo::globalToWorld(const RCVector3f& point) const
{
	return ((mToWorldFromGlobal * point.convertTo<double>()) + mGeoReference).convertTo<float>();
}

///////////////////////////////////////////////////////////////////////////////
// \brief: Global -> World
///////////////////////////////////////////////////////////////////////////////
RCBox AgWorldCoordInfo::globalToWorld(const RCBox& box) const
{
	RCBox worldBox = box.getTransformed(mToWorldFromGlobal);

	//then add Geo Reference
	worldBox.setMin(worldBox.getMin() + mGeoReference);
	worldBox.setMax(worldBox.getMax() + mGeoReference);
	return worldBox;
}

RCOrientedBoundingBox AgWorldCoordInfo::globalToWorld(const RCOrientedBoundingBox& box) const
{
	auto t = globalToWorld(box.getTransform());
	return RCOrientedBoundingBox(t, box.getSize());
}

RCTransform AgWorldCoordInfo::globalToWorld(const RCTransform& transform) const
{
	auto t = transform;
	t = mToWorldFromGlobal * t;
	t.translate(mGeoReference);
	return t;
}

///////////////////////////////////////////////////////////////////////////////
// \brief: Global -> GeoReferencedGlobal
///////////////////////////////////////////////////////////////////////////////
RCVector3d AgWorldCoordInfo::globalToGeoGlobal(const RCVector3d& point) const
{
	return worldToGeoGlobal(globalToWorld(point));
}

///////////////////////////////////////////////////////////////////////////////
// \brief: Global -> GeoReferencedGlobal
///////////////////////////////////////////////////////////////////////////////
RCBox AgWorldCoordInfo::globalToGeoGlobal(const RCBox& box) const
{
	return worldToGeoGlobal(globalToWorld(box));
}

RCOrientedBoundingBox AgWorldCoordInfo::globalToGeoGlobal(const RCOrientedBoundingBox& box) const
{
	return worldToGeoGlobal(globalToWorld(box));
}

RCTransform AgWorldCoordInfo::globalToGeoGlobal(const RCTransform& transform) const
{
	return worldToGeoGlobal(globalToWorld(transform));
}

///////////////////////////////////////////////////////////////////////////////
// \brief: GeoReferencedGlobal -> Global
///////////////////////////////////////////////////////////////////////////////
RCVector3d AgWorldCoordInfo::geoGlobalToGlobal(const RCVector3d& point) const
{
	return worldToGlobal(geoGlobalToWorld(point));
}

///////////////////////////////////////////////////////////////////////////////
// \brief: GeoReferencedGlobal -> Global
///////////////////////////////////////////////////////////////////////////////
RCBox AgWorldCoordInfo::geoGlobalToGlobal(const RCBox& box) const
{
	return worldToGlobal(geoGlobalToWorld(box));
}

RCOrientedBoundingBox AgWorldCoordInfo::geoGlobalToGlobal(const RCOrientedBoundingBox& box) const
{
	return worldToGlobal(geoGlobalToWorld(box));
}

RCTransform AgWorldCoordInfo::geoGlobalToGlobal(const RCTransform& transform) const
{
	return worldToGlobal(geoGlobalToWorld(transform));
}

///////////////////////////////////////////////////////////////////////////////
// \brief: World Direction -> Global Direction
///////////////////////////////////////////////////////////////////////////////
RCVector3d AgWorldCoordInfo::worldDirection2GlobalDirection(const RCVector3d& dir) const
{
	RCVector3d orgin(0.0, 0.0, 0.0);
	return (worldToGlobal(dir) - worldToGlobal(orgin)).normalize();
}

///////////////////////////////////////////////////////////////////////////////
// \brief: Global Direction -> World Direction
///////////////////////////////////////////////////////////////////////////////
RCVector3d AgWorldCoordInfo::globalDirection2worldDirection(const RCVector3d& dir) const
{
	RCVector3d orgin(0.0, 0.0, 0.0);
	return (globalToWorld(dir) - globalToWorld(orgin)).normalize();
}