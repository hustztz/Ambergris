$input v_normal, v_texcoord0

/*
 * Copyright 2011-2018 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include "common/common.sh"

uniform vec4 u_lightDir;

SAMPLER2D(s_texColor,  0);

void main()
{
	vec3 normal = normalize(v_normal);
	vec3 ambient = vec3(0.3, 0.3, 0.3);
	float diffuse = max(0.0, dot(normal, u_lightDir.xyz));

	vec3 color = toLinear(texture2D(s_texColor, v_texcoord0) );

	//gl_FragColor.xyz = pow(vec3(0.07, 0.06, 0.08) + color*lc.y + fres*pow(lc.z, 128.0), vec3_splat(1.0/2.2) );
	gl_FragColor.xyz = pow((ambient + diffuse) * color, vec3_splat(1.0/2.2) );
	gl_FragColor.w = 1.0;
}
