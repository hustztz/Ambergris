vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 1.0);
vec4 v_color0    : COLOR0    = vec4(1.0, 0.0, 0.0, 1.0);
vec2 v_texcoord0 : TEXCOORD0 = vec2(0.0, 0.0);

vec4 v_texcoord1 : TEXCOORD1 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_texcoord2 : TEXCOORD2 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_texcoord3 : TEXCOORD3 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_texcoord4 : TEXCOORD4 = vec4(0.0, 0.0, 0.0, 0.0);
vec3 v_view      : TEXCOORD5 = vec3(0.0, 0.0, 0.0);
vec4 v_shadowcoord : TEXCOORD6 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_position    : TEXCOORD7 = vec4(0.0, 0.0, 0.0, 0.0);
float v_depth      : FOG       = 0.0;

vec2 v_screenPos : TEXCOORD0 = vec2(0.0, 0.0);
vec3 v_skyColor  : TEXCOORD1 = vec3(0.0, 0.0, 1.0);
vec3 v_viewDir   : TEXCOORD2 = vec3(0.0, 0.0, 1.0);

vec3 a_position  : POSITION;
vec2 a_texcoord0 : TEXCOORD0;
vec3 a_normal    : NORMAL;
vec4 i_data0     : TEXCOORD7;
vec4 i_data1     : TEXCOORD6;
vec4 i_data2     : TEXCOORD5;
vec4 i_data3     : TEXCOORD4;
