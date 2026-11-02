#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

uniform vec2 outputSize;
out vec2 glyphUV;

void main()
{
	gl_Position = vec4(position / outputSize * vec2(2.0, -2.0) + vec2(-1.0, 1.0), 0.0, 1.0);
	glyphUV = uv;
}
