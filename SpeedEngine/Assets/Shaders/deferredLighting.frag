#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;
uniform sampler2D gEmissive;

uniform bool useDirLight;
uniform vec3 dirLightDirection;
uniform vec3 dirLightColor;

#define MAX_LIGHTS 16
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 camPos;

uniform float ambientStrength;
uniform float gamma;

void main()
{
    // TODO: lighting, outputs unlit albedo for now
    FragColor = vec4(pow(texture(gAlbedo, TexCoords).rgb, vec3(1.0 / gamma)), 1.0);
}
