#include "AgDynamicValueController.h"

namespace ambergris {

	// HDTV rec. 709 matrix.
	static float M_XYZ2RGB[] =
	{
		3.240479f, -0.969256f, 0.055648f,
		-1.53715f, 1.875991f, -0.204043f,
		-0.49853f, 0.041556f, 1.057311f
	};


	// Converts color repesentation from CIE XYZ to RGB color-space.
	AgDynamicValueController::Color AgDynamicValueController::XYZToRGB(const AgDynamicValueController::Color& xyz)
	{
		AgDynamicValueController::Color rgb;
		rgb.r = M_XYZ2RGB[0] * xyz.X + M_XYZ2RGB[3] * xyz.Y + M_XYZ2RGB[6] * xyz.Z;
		rgb.g = M_XYZ2RGB[1] * xyz.X + M_XYZ2RGB[4] * xyz.Y + M_XYZ2RGB[7] * xyz.Z;
		rgb.b = M_XYZ2RGB[2] * xyz.X + M_XYZ2RGB[5] * xyz.Y + M_XYZ2RGB[8] * xyz.Z;
		return rgb;
	};
}