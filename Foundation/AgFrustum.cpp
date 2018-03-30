#include "AgFrustum.h"

#include <common/RCVector.h>
#include <common/RCBox.h>
#include <common/RCTransform.h>

#define TOP 0
#define BOTTOM 1
//#define LEFT 2
//#define RIGHT 3
#define FRONT 4
#define BACK 5

using namespace ambergris;
using namespace ambergris::RealityComputing::Common;

void AgFrustum::setFrustum(const RCProjection &projMVMatrix)
{
	double clip[16];
	projMVMatrix.toColumnMajor(clip);

	planeList[FRUST_RIGHT] = RCPlane(
		RCVector3d(clip[3] - clip[0], clip[7] - clip[4], clip[11] - clip[8]), (clip[15] - clip[12]));

	planeList[FRUST_LEFT] = RCPlane(
		RCVector3d(clip[3] + clip[0], clip[7] + clip[4], clip[11] + clip[8]), (clip[15] + clip[12]));

	planeList[FRUST_BOTTOM] = RCPlane(
		RCVector3d(clip[3] + clip[1], clip[7] + clip[5], clip[11] + clip[9]), (clip[15] + clip[13]));

	planeList[FRUST_TOP] = RCPlane(
		RCVector3d(clip[3] - clip[1], clip[7] - clip[5], clip[11] - clip[9]), (clip[15] - clip[13]));

	planeList[FRUST_NEAR] = RCPlane(
		RCVector3d(clip[3] + clip[2], clip[7] + clip[6], clip[11] + clip[10]), (clip[15] + clip[14]));


	planeList[FRUST_FAR] = RCPlane(
		RCVector3d(clip[3] - clip[2], clip[7] - clip[6], clip[11] - clip[10]), (clip[15] - clip[14]));

	//planeList[FRUST_FAR].setNormal( RCVector3d(0.0));

	double lenght = planeList[FRUST_FAR].getNormal().lengthSqrd();
	if (lenght == 0.0) {
		mFarPlaneInfinite = true;
		mNumValidPlanes = 5;
	}


	for (int i = 0; i < mNumValidPlanes; i++)
	{
		planeList[i].normalize();
	}
	//cannot calculat intersections with far plane

	calcClippingPoints();
}

void AgFrustum::calcClippingPoints()
{
	//far plane top left
	if (!mFarPlaneInfinite) //cannot intersect
	{
		planeList[FRUST_LEFT].intersect(planeList[TOP], planeList[BACK], farPlanePoints[0]);
		planeList[FRUST_RIGHT].intersect(planeList[TOP], planeList[BACK], farPlanePoints[1]);
		planeList[FRUST_RIGHT].intersect(planeList[BOTTOM], planeList[BACK], farPlanePoints[2]);
		planeList[FRUST_LEFT].intersect(planeList[BOTTOM], planeList[BACK], farPlanePoints[3]);
	}

	planeList[FRUST_LEFT].intersect(planeList[TOP], planeList[FRONT], farPlanePoints[4]);
	planeList[FRUST_RIGHT].intersect(planeList[TOP], planeList[FRONT], farPlanePoints[5]);
	planeList[FRUST_RIGHT].intersect(planeList[BOTTOM], planeList[FRONT], farPlanePoints[6]);
	planeList[FRUST_LEFT].intersect(planeList[BOTTOM], planeList[FRONT], farPlanePoints[7]);


	rightDir = (farPlanePoints[1] - farPlanePoints[0]).convertTo<float>();
	rightLength = static_cast<float>(rightDir.length());
	rightDir.normalize();

	leftFrustum = farPlanePoints[0].convertTo<float>();

	downDir = (farPlanePoints[3] - farPlanePoints[0]).convertTo<float>();
	downLength = static_cast<float>(downDir.length());
	downDir.normalize();


}

bool AgFrustum::sphereInFrustum(const RCVector3d& position, double radius) const
{
	for (int i = 0; i < mNumValidPlanes; i++)
	{
		if (planeList[i].distanceToPlane(position) <= -radius)
			return false;
	}
	return true;
}




void AgFrustum::setFrustum(const RCProjection & proj, const RCTransform & model)
{
	setFrustum(proj * model);
}


int AgFrustum::BoxInFrustum(const RCBox& box) const
{
	return BoxInfrustumLazy(box);
}

bool AgFrustum::BoxInfrustumLazy(const RCBox & bounds) const
{
	RCVector3d tmp[8];
	getCorners(bounds, tmp);
	for (int i = 0; i < mNumValidPlanes; i++)
	{
		bool valid = true;
		for (int j = 0; j < 8; j++)
		{
			if (planeList[i].distanceToPlane(tmp[j]) > 0.01)
			{
				valid = false;
				break;
			}
		}
		if (valid) {

			return false;
		}
	}
	return true;
}


bool AgFrustum::transformedBoxInFrustum(const RCBox& box, const RCTransform& transform) const
{
	RCVector3d tmp[8];
	getCorners(box, tmp);
	for (int i = 0; i < mNumValidPlanes; i++)
	{
		bool valid = true;
		for (int j = 0; j < 8; j++)
		{
			RCVector3d transPt = transform * tmp[j];
			if (planeList[i].distanceToPlane(transPt) > 0.01)
			{
				valid = false;
				break;
			}
		}
		if (valid) {

			return false;
		}
	}
	return true;
}


bool AgFrustum::pointInFrustum(const RCVector3d & point) const
{
	for (int i = 0; i < mNumValidPlanes; i++)
	{
		if (planeList[i].distanceToPlane(point) <= 0.0)
			return false;
	}
	return true;
}

bool AgFrustum::pointInFrustum(const RCVector3f & point) const
{
	return pointInFrustum(point.convertTo<double>());
}

void AgFrustum::getRayDir(RCVector3f &ray, float stepX, float stepY)
{
	ray = leftFrustum + (rightDir * (stepX * rightLength)) + (downDir * (stepY * downLength));

}

RCPlane* AgFrustum::getPlaneList()
{
	return &planeList[0];
}

const RCPlane& AgFrustum::getPlaneAt(int index) const
{
	return planeList[index];
}

RCVector3d* AgFrustum::getFarClipPoints()
{
	return &farPlanePoints[0];
}
