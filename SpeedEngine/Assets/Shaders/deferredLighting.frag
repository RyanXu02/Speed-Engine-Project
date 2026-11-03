#version 330 core

out vec4 FragColor;

in vec2 TexCoords;

// material factors setup is similar to the forward pass
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D gMaterial;
uniform sampler2D gEmissive;

// material factors
uniform vec4 albedoFactor;
uniform float metallicFactor;
uniform float roughnessFactor;
uniform float aoStrength;
uniform float normalScale;
uniform vec3 emissiveFactor;

// single directional light, kept separate for shadow mapping later
uniform bool useDirLight;
uniform vec3 dirLightDirection;
uniform vec3 dirLightColor;

// shadow map from the ShadowRenderer pass
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform float shadowBias;
uniform float shadowNormalOffset;

// point lights
#define MAX_LIGHTS 16
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 camPos;

// tunables
uniform float ambientStrength;
uniform float gamma;

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// outgoing radiance from one light, the body of the light loop in the article
vec3 calculateLo(vec3 N, vec3 V, vec3 L, vec3 radiance, vec3 F0, vec3 albedo, float metallic, float roughness)
{
    vec3 H = normalize(V + L);

    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular     = numerator / denominator;

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * radiance * NdotL;
}

float calculateShadow(vec3 worldPos, vec3 N, vec3 L)
{
    float normalOffset = shadowNormalOffset * (1.0 - max(dot(N, L), 0.0));
    vec4 lightSpacePos = lightSpaceMatrix * vec4(worldPos + N * normalOffset, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    vec3 projCoordsNDC = projCoords * 0.5 + 0.5;

    float closestDepth = texture(shadowMap, projCoordsNDC.xy).r;
    float shadow = projCoordsNDC.z > closestDepth ? 0.0 : 1.0;
    if(projCoordsNDC.z > closestDepth) {
        shadow = 0.0;
    } else {
        shadow = 1.0;
    }

    return shadow;

}

void main()
{
    float ao = texture(gMaterial, TexCoords).r;
    float roughness = texture(gMaterial, TexCoords).g;
    float metallic = texture(gMaterial, TexCoords).b;
    vec3 emissive = texture(gEmissive, TexCoords).rgb;

    vec3 WorldPos = texture(gPosition, TexCoords).xyz;
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    float alpha = texture(gAlbedo, TexCoords).a;
    
    vec3 N = normalize(texture(gNormal, TexCoords).xyz);
    vec3 V = normalize(camPos - WorldPos);
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    if (useDirLight)
    {
        vec3 L = normalize(-dirLightDirection);
        float shadow = calculateShadow(WorldPos, N, L);
        Lo += shadow * calculateLo(N, V, L, dirLightColor, F0, albedo, metallic, roughness);
    }

    for(int i = 0; i < numLights && i < MAX_LIGHTS; ++i)
    {
        vec3 L = normalize(lightPositions[i] - WorldPos);
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        Lo += calculateLo(N, V, L, radiance, F0, albedo, metallic, roughness);
    }

    vec3 ambient = vec3(ambientStrength) * albedo * ao;
    vec3 color   = ambient + Lo + emissive;

    // TODO: seprate Tonemap and gamma correct in to a finalize pass to not lose detail for inbetween passes
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / gamma));

    FragColor = vec4(color, 1.0);
}
