$input v_color0
/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	gl_FragColor.xyz = v_color0.xyz; // This is dumb, should use u8 texture
	gl_FragColor.w = 1.0;
}
