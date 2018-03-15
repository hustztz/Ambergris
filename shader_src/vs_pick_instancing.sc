$input a_position, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_color0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	mat4 instMtx;
	instMtx[0] = i_data0;
	instMtx[1] = i_data1;
	instMtx[2] = i_data2;
	instMtx[3] = i_data3;

	vec4 instancePos = instMul(instMtx, vec4(a_position, 1.0) );
	gl_Position = mul(u_viewProj, vec4(instancePos.xyz, 1.0) );
	v_color0 = i_data4;
}
