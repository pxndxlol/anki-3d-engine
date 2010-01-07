#pragma anki vert_shader_begins

attribute vec3 position;
attribute vec3 normal;
attribute vec2 tex_coords;

varying vec2 tex_coords_v2f;
varying vec3 normal_v2f;

void main()
{
	tex_coords_v2f = tex_coords;
	normal_v2f = gl_NormalMatrix * normal;
	gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}

#pragma anki frag_shader_begins

#pragma anki include "shaders/pack.glsl"

uniform sampler2D diffuse_map, noise_map;
varying vec2 tex_coords_v2f;
varying vec3 normal_v2f;

void main()
{
	/*vec3 _noise = DecodeNormal( texture2D( noise_map, gl_FragCoord.xy*vec2( 1.0/R_W, 1.0/R_H ) ).rg ).rgb;
	_noise = _noise * 2 - 1;
	_noise *= 7.0;*/

	vec4 _texel = texture2D( diffuse_map, (gl_FragCoord.xy+(normal_v2f.xy*2))*vec2( 1.0/R_W, 1.0/R_H ) );


	//vec4 _texel = texture2D( diffuse_map, tex_coords_v2f ) / 2;

	gl_FragData[0] = _texel;
}