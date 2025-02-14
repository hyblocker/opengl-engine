precision mediump float;

#include "common.glsl"

// gl_FragColor is deprecated in GLSL 4.4+
layout(location = 0) out vec4 fragColor;
layout(binding = 0) uniform sampler2D brickTex;

in vec3 eyeDir;

#define SUN_DIR normalize(light0.direction)

// Star Nest by Pablo Roman Andrioli
// License: MIT

#define iterations 17
#define formuparam 0.53

#define volsteps 20
#define stepsize 0.1

#define zoom   0.800
#define tile   0.850
#define speed  0.005 

#define brightness 0.0015
#define darkmatter 0.300
#define distfading 0.730
#define saturation 0.850

void mainImage( out vec4 fragColor, in vec3 eyeVector )
{
	//get coords and direction
	// vec3 dir=normalize(eyeVector+vec3(0,3,5));
	vec3 dir=eyeVector;
	vec3 from=cameraPos + vec3(50,100,20) + vec3(0.8 * elapsedTime * speed, elapsedTime * speed, -0.02 * elapsedTime * speed);
	
	//volumetric rendering
	float s=0.1,fade=1.;
	vec3 v=vec3(0.);
	for (int r=0; r<volsteps; r++) {
		vec3 p=from+s*dir*.5;
		p = abs(vec3(tile)-mod(p,vec3(tile*2.))); // tiling fold
		float pa,a=pa=0.;
		for (int i=0; i<iterations; i++) { 
			p=abs(p)/dot(p,p)-formuparam; // the magic formula
			a+=abs(length(p)-pa); // absolute sum of average change
			pa=length(p);
		}
		float dm=max(0.,darkmatter-a*a*.001); //dark matter
		a*=a*a; // add contrast
		if (r>6) fade*=1.-dm; // dark matter, don't render near
		//v+=vec3(dm,dm*.5,0.);
		v+=fade;
		v+=vec3(s,s*s,s*s*s*s)*a*brightness*fade; // coloring based on distance
		fade*=distfading; // distance fading
		s+=stepsize;
	}
	v=mix(vec3(length(v)),v,saturation); //color adjust
	fragColor = vec4(v*.01,1.);	
	
}

void main()
{
    vec3 direction = normalize(eyeDir.xyz);
    // debug eye vector
    // fragColor = vec4(direction.xyz * 0.5f + 0.5f, 1);
    vec4 outColor = vec4(0,0,0,1);
    mainImage(outColor, direction);
    fragColor = outColor;

}