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

uniform sampler2D terrainSplatMap;
uniform sampler2D terrainHeightMap;

uniform sampler2D albedoMap, albedoMap1, albedoMap2;
uniform sampler2D normalMap, normalMap1, normalMap2;
uniform sampler2D metallicMap, metallicMap1, metallicMap2;
uniform sampler2D roughnessMap, roughnessMap1, roughnessMap2;
uniform sampler2D aoMap, aoMap1, aoMap2;
uniform sampler2D emissiveMap, emissiveMap1, emissiveMap2;

// material controls
uniform vec4 albedoFactor0, albedoFactor1, albedoFactor2;
uniform float metallicFactor0, metallicFactor1, metallicFactor2;
uniform float roughnessFactor0, roughnessFactor1, roughnessFactor2;
uniform float aoStrength0, aoStrength1, aoStrength2;
uniform float normalScale0, normalScale1, normalScale2;
uniform vec3 emissiveFactor0, emissiveFactor1, emissiveFactor2;

const float texScale = 50.0;

vec3 filterNormal(vec2 uv, float texelSize)
{
    vec4 h;
    h[0] = texture(terrainHeightMap, uv + texelSize*vec2( 0,-1)).r;
    h[1] = texture(terrainHeightMap, uv + texelSize*vec2(-1, 0)).r;
    h[2] = texture(terrainHeightMap, uv + texelSize*vec2( 1, 0)).r;
    h[3] = texture(terrainHeightMap, uv + texelSize*vec2( 0, 1)).r;
    vec3 n;
    n.z = h[0] - h[3];
    n.x = h[1] - h[2];
    n.y = 2;

    return normalize(n);
} 

vec3 getNormalFromMap(vec3 mixAmt)
{
    vec3 tangentNormal0 = texture(normalMap, TexCoords * texScale).xyz * 2.0 - 1.0;
    tangentNormal0.xy *= normalScale0;
    vec3 tangentNormal1 = texture(normalMap1, TexCoords * texScale).xyz * 2.0 - 1.0;
    tangentNormal1.xy *= normalScale1;
    vec3 tangentNormal2 = texture(normalMap2, TexCoords * texScale).xyz * 2.0 - 1.0;
    tangentNormal2.xy *= normalScale2;

    vec3 tangentNormal = tangentNormal0 * mixAmt.x + tangentNormal1 * mixAmt.y + tangentNormal2 * mixAmt.z;


    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = filterNormal(TexCoords, 1.0 / 2017.0);
    vec3 T   = normalize(Q1 * st2.t - Q2 * st1.t);
    vec3 B   = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


void main()
{
    vec3 mixAmount = texture(terrainSplatMap, TexCoords).xyz;
    // Normalize
    mixAmount = mixAmount / (mixAmount.x + mixAmount.y + mixAmount.z);

    vec4 albedoSample0 = texture(albedoMap, TexCoords * texScale);
    vec4 albedoSample1 = texture(albedoMap1, TexCoords * texScale);
    vec4 albedoSample2 = texture(albedoMap2, TexCoords * texScale);
    vec3 albedo0    = pow(albedoSample0.rgb, vec3(2.2)) * albedoFactor0.rgb;
    vec3 albedo1    = pow(albedoSample1.rgb, vec3(2.2)) * albedoFactor1.rgb;
    vec3 albedo2    = pow(albedoSample2.rgb, vec3(2.2)) * albedoFactor2.rgb;
    vec3 albedo = albedo0 * mixAmount.x + albedo1 * mixAmount.y + albedo2 * mixAmount.z;

    float ao0       = mix(1.0, texture(aoMap, TexCoords * texScale).r, aoStrength0);
    float ao1       = mix(1.0, texture(aoMap1, TexCoords * texScale).r, aoStrength1);
    float ao2       = mix(1.0, texture(aoMap2, TexCoords * texScale).r, aoStrength2);
    float ao        = ao0 * mixAmount.x + ao1 * mixAmount.y + ao2 * mixAmount.z;

    float roughness0 = max(texture(roughnessMap, TexCoords * texScale).g * roughnessFactor0, 0.04);
    float roughness1 = max(texture(roughnessMap1, TexCoords * texScale).g * roughnessFactor1, 0.04);
    float roughness2 = max(texture(roughnessMap2, TexCoords * texScale).g * roughnessFactor2, 0.04);
    float roughness = roughness0 * mixAmount.x + roughness1 * mixAmount.y + roughness2 * mixAmount.z;

    float metallic0  = texture(metallicMap, TexCoords * texScale).b * metallicFactor0;
    float metallic1  = texture(metallicMap1, TexCoords * texScale).b * metallicFactor1;
    float metallic2  = texture(metallicMap2, TexCoords * texScale).b * metallicFactor2;
    float metallic = metallic0 * mixAmount.x + metallic1 * mixAmount.y + metallic2 * mixAmount.z;

    vec3 emissive0   = pow(texture(emissiveMap, TexCoords * texScale).rgb, vec3(2.2)) * emissiveFactor0;
    vec3 emissive1   = pow(texture(emissiveMap1, TexCoords * texScale).rgb, vec3(2.2)) * emissiveFactor1;
    vec3 emissive2   = pow(texture(emissiveMap2, TexCoords * texScale).rgb, vec3(2.2)) * emissiveFactor2;
    vec3 emissive = emissive0 * mixAmount.x + emissive1 * mixAmount.y + emissive2 * mixAmount.z;

    gPosition = vec4(WorldPos, 1.0);
    gNormal   = vec4(getNormalFromMap(mixAmount), 0.0);
    gAlbedo   = vec4(albedo, 0.0);
    gMaterial = vec4(ao, roughness, metallic, 0.0);
    gEmissive = vec4(emissive, 0.0);
}
