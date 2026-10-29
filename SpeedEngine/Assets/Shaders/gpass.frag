#version 330 core

// G-buffer layout, must match the GBuffer target defined in the cpp side
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedo;   // rgb = albedo, a = ambient occlusion
layout (location = 3) out vec4 gMaterial; // r = metallic, g = roughness
layout (location = 4) out vec4 gEmissive;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D emissiveMap;

// material controls
uniform vec4 albedoFactor;
uniform float metallicFactor;
uniform float roughnessFactor;
uniform float aoStrength;
uniform float normalScale;
uniform vec3 emissiveFactor;

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalMap, TexCoords).xyz * 2.0 - 1.0;
    tangentNormal.xy *= normalScale;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(Normal);
    vec3 T   = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main()
{
    vec4 albedoSample = texture(albedoMap, TexCoords);
    vec3 albedo    = pow(albedoSample.rgb, vec3(2.2)) * albedoFactor.rgb;
    float metallic  = texture(metallicMap, TexCoords).b * metallicFactor;
    float roughness = max(texture(roughnessMap, TexCoords).g * roughnessFactor, 0.04);
    float ao        = mix(1.0, texture(aoMap, TexCoords).r, aoStrength);
    vec3 emissive   = pow(texture(emissiveMap, TexCoords).rgb, vec3(2.2)) * emissiveFactor;

    gPosition = vec4(WorldPos, 1.0);
    gNormal   = vec4(getNormalFromMap(), 0.0);
    gAlbedo   = vec4(albedo, ao);
    gMaterial = vec4(metallic, roughness, 0.0, 0.0);
    gEmissive = vec4(emissive, 0.0);
}
