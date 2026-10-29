#version 330 core

// pbr lighting setup, direct reference from https://learnopengl.com/PBR/Lighting

out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;

// material maps, missing maps are bound to neutral 1x1 textures
uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D emissiveMap;

// material factors (glTF)
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

void main()
{
    vec4 albedoSample = texture(albedoMap, TexCoords);
    vec3 albedo       = pow(albedoSample.rgb, vec3(2.2)) * albedoFactor.rgb;
    float alpha       = albedoSample.a * albedoFactor.a;

    // glTF's roughness value is in g channel, while metallic is in b channel
    float metallic  = texture(metallicMap, TexCoords).b * metallicFactor;
    float roughness = max(texture(roughnessMap, TexCoords).g * roughnessFactor, 0.04);
    float ao        = mix(1.0, texture(aoMap, TexCoords).r, aoStrength);
    vec3 emissive   = pow(texture(emissiveMap, TexCoords).rgb, vec3(2.2)) * emissiveFactor;

    vec3 N = getNormalFromMap();
    vec3 V = normalize(camPos - WorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

    if (useDirLight)
    {
        vec3 L = normalize(-dirLightDirection);
        Lo += calculateLo(N, V, L, dirLightColor, F0, albedo, metallic, roughness);
    }

    for (int i = 0; i < numLights && i < MAX_LIGHTS; ++i)
    {
        vec3 L = normalize(lightPositions[i] - WorldPos);
        float distance    = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = lightColors[i] * attenuation;

        Lo += calculateLo(N, V, L, radiance, F0, albedo, metallic, roughness);
    }

    vec3 ambient = vec3(ambientStrength) * albedo * ao;
    vec3 color   = ambient + Lo + emissive;

    // TODO: seprate Tonemap and gamma correct in to a finalize pass to not lose detail for inbetween passes
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / gamma));

    FragColor = vec4(color, alpha);

    if (FragColor.a < 0.01)
        discard;
}
