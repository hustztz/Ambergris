$input a_position, a_normal, a_texcoord0
$output v_normal, v_texcoord0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	vec3 wpos = mul(u_model[0], vec4(a_position, 1.0) ).xyz;
	gl_Position = mul(u_viewProj, vec4(wpos, 1.0) );
	
	v_normal = mul(u_modelView, vec4(a_normal.xyz, 0.0) ).xyz;
	
	v_texcoord0 = a_texcoord0;
}