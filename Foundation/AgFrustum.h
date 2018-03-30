#pragma once

#include <common/RCPlane.h>

namespace ambergris {

	namespace RealityComputing {
		namespace Common {
			class RCTransform;
			class RCProjection;
			struct RCBox;
		}
	}

	class AgFrustum
	{
	public:
		enum
		{
			FRUST_TOP = 0,
			FRUST_BOTTOM,
			FRUST_LEFT,
			FRUST_RIGHT,
			FRUST_NEAR,
			FRUST_FAR
		};
	public:

		AgFrustum()
		{
			mFarPlaneInfinite = false;
			mNumValidPlanes = 6;
		}

		~AgFrustum()
		{

		}

		AgFrustum& operator=(const AgFrustum&) = default;

		void                     setFrustum(const RealityComputing::Common::RCProjection & proj, const RealityComputing::Common::RCTransform& model);

		void                        setFrustum(const RealityComputing::Common::RCProjection & projMVMatrix);
		//////////////////////////////////////////////////////////////////////////
		// \brief: Returns if the far plane is infinite
		//////////////////////////////////////////////////////////////////////////
		bool                        isInfiniteFarPlane() const;


		// Call this every time the camera moves to update the frustum
		void                        CalculateFrustum();

		// This takes a 3D point and returns TRUE if it's inside of the frustum
		bool                        pointInFrustum(const RealityComputing::Common::RCVector3d & point) const;
		bool                        pointInFrustum(const RealityComputing::Common::RCVector3f & point) const;


		// This takes a 3D point and a radius and returns TRUE if the sphere is inside of the frustum
		// int sphereInFrustum(double* center, double radius);

		// This takes the center and half the length of the cube.
		bool                        CubeInFrustum(double x, double y, double z, double size);

		RealityComputing::Common::RCPlane*            getPlaneList();

		const RealityComputing::Common::RCPlane&      getPlaneAt(int index) const;


		bool                        sphereInFrustum(const RealityComputing::Common::RCVector3d&  position, double radius)const;

		int                         BoxInFrustum(const RealityComputing::Common::RCBox& box) const;

		bool                        BoxInfrustumLazy(const RealityComputing::Common::RCBox& bounds)const;

		bool                        transformedBoxInFrustum(const RealityComputing::Common::RCBox& box,
			const RealityComputing::Common::RCTransform& transform) const;

		void                        calcClippingPoints();

		RealityComputing::Common::RCVector3d*         getFarClipPoints();

		void                        getRayDir(RealityComputing::Common::RCVector3f &ray, float stepX, float stepY);



		RealityComputing::Common::RCVector3d          farPlanePoints[8];
		RealityComputing::Common::RCPlane         planeList[6];

		RealityComputing::Common::RCVector3f          leftFrustum;
		RealityComputing::Common::RCVector3f          rightDir;
		RealityComputing::Common::RCVector3f          downDir;
		float                       rightLength;
		float                       downLength;
		bool                        mFarPlaneInfinite;
		int                         mNumValidPlanes;




		double angle;
		double ratio;
		double sphereFactorX, sphereFactorY;
		double tang;
		double nw, nh, fw, fh;
		double nearClipPlane, farClipPlane;
	};
}