#version 330 core

layout(location = 0) in vec2 corner;     // per-vertex quad corner in [-0.5, 0.5]
layout(location = 1) in vec3 position;   // per-instance
layout(location = 2) in vec2 size;
layout(location = 3) in float rotation;
layout(location = 4) in vec4 color;

uniform mat4 view;
uniform mat4 projectionView;

out vec2 uv;
out vec4 particleColor;

void main()
{
	vec3 right = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 up = vec3(view[0][1], view[1][1], view[2][1]);

	float c = cos(rotation);
	float s = sin(rotation);
	vec2 rotated = vec2(corner.x * c - corner.y * s, corner.x * s + corner.y * c);

	vec3 worldPos = position + (right * rotated.x *size.x + up * rotated.y*size.y); //this makes quad always face camera
	gl_Position = projectionView * vec4(worldPos, 1.0);
	uv = corner + 0.5;
	particleColor = color;
}
