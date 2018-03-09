$input a_position, i_data0, i_data1, i_data2, i_data3
$output v_color0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

uniform vec4 u_id;

void main()
{
	mat4 instMtx;
	instMtx[0] = i_data0;
	instMtx[1] = i_data1;
	instMtx[2] = i_data2;
	instMtx[3] = i_data3;

	vec4 instancePos = instMul(instMtx, vec4(a_position, 1.0) );
	gl_Position = mul(u_modelViewProj, instancePos );
	v_color0 = u_id;
}
