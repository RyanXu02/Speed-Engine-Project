#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 projectionView;

void main()
{
    vec3 WorldPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projectionView * vec4(WorldPos, 1.0);
}
