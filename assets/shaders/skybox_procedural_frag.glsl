precision mediump float;

#include "common.glsl"

// gl_FragColor is deprecated in GLSL 4.4+
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D brickTex;

in vec3 eyeDir;

#define SUN_DIR normalize(light0.direction)

//
// Fast skycolor function by Íñigo Quílez
// https://www.shadertoy.com/view/MdX3Rr
//
vec3 getSkyColor(vec3 rd) {
    float sundot = clamp(dot(rd,SUN_DIR),0.0,1.0);
	vec3 col = vec3(0.2,0.5,0.85)*1.1 - max(rd.y,0.01)*max(rd.y,0.01)*0.5;
    col = mix( col, 0.85*vec3(0.7,0.75,0.85), pow(1.0-max(rd.y,0.0), 6.0) );

    col += 0.25*vec3(1.0,0.7,0.4)*pow( sundot,5.0 );
    col += 0.25*vec3(1.0,0.8,0.6)*pow( sundot,64.0 );
    col += 0.20*vec3(1.0,0.8,0.6)*pow( sundot,512.0 );
    
    col += clamp((0.1-rd.y)*10., 0., 1.) * vec3(.0,.1,.2);
    col += 0.2*vec3(1.0,0.8,0.6)*pow( sundot, 8.0 );
    return col;
}


void main()
{
    vec3 direction = normalize(eyeDir.xyz);
    // debug eye vector
    // fragColor = vec4(direction.xyz * 0.5f + 0.5f, 1);
    fragColor = vec4(getSkyColor(direction.xyz), 1);

}
