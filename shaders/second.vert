#version 330

// The normal map shader

layout(location = 0) in vec3 position;

out vec2 texpos;

uniform mat4 mdvMat;
uniform mat4 projMat;
uniform sampler2D heightmap;

void main() {
	vec4 height = texture(heightmap,position.xy*0.5+0.5);
	gl_Position = projMat*mdvMat*vec4(position.xy, height.x, 1);

	texpos = position.xy*0.5+0.5;
}
