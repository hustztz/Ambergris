$input a_position, i_data0, i_data1, i_data2, i_data3
$output v_color0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	mat4 instMtx;
	instMtx[0] = vec4(i_data0.xyz, 0.0);
	instMtx[1] = vec4(i_data1.xyz, 0.0);
	instMtx[2] = vec4(i_data2.xyz, 0.0);
	instMtx[3] = i_data3;

	vec4 instancePos = instMul(instMtx, vec4(a_position, 1.0) );
	gl_Position = mul(u_viewProj, vec4(instancePos.xyz, 1.0) );
	v_color0 = vec4(i_data0.w, i_data1.w, i_data2.w, 1.0f);
}
