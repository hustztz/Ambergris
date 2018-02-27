$input a_position, a_normal, a_texcoord0
$output v_wpos, v_view, v_normal, v_texcoord0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "../common/common.sh"

void main()
{
	v_wpos = mul(u_model[0], vec4(a_position, 1.0) ).xyz;
	gl_Position = mul(u_viewProj, vec4(v_wpos, 1.0) );
	
	vec4 normal = a_normal * 2.0 - 1.0;
	v_normal = mul(u_modelView, vec4(normal.xyz, 0.0) ).xyz;
	
	v_texcoord0 = a_texcoord0;
}
