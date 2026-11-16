#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 projectionView;

uniform float heightScale;

uniform sampler2D terrainHeightMap;

void main()
{
    vec3 positionFinal = aPos + vec3(0.0, texture(terrainHeightMap, aTexCoords).x * heightScale ,0.0);
    TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(positionFinal, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;

    gl_Position = projectionView * vec4(WorldPos, 1.0);
}
