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

// point lights
#define MAX_LIGHTS 16
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 camPos;

// tunables
uniform float ambientStrength;

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

// image based lighting from the ImageBasedRenderer, falls back to the flat ambient when off
uniform bool useIBL;
uniform vec3 shIrradiance[9];       // irradiance SH, already cosine convolved and divided by PI
uniform samplerCube prefilterMap;   // GGX prefiltered environment, mip = roughness * prefilterMaxLod
uniform sampler2D brdfLUT;          // split sum scale and bias on F0
uniform float prefilterMaxLod;
uniform float iblIntensity;

// same basis as iblSHProject.comp
vec3 evaluateSH(vec3 n)
{
    return shIrradiance[0] * 0.282095
         + shIrradiance[1] * 0.488603 * n.y
         + shIrradiance[2] * 0.488603 * n.z
         + shIrradiance[3] * 0.488603 * n.x
         + shIrradiance[4] * 1.092548 * n.x * n.y
         + shIrradiance[5] * 1.092548 * n.y * n.z
         + shIrradiance[6] * 0.315392 * (3.0 * n.z * n.z - 1.0)
         + shIrradiance[7] * 1.092548 * n.x * n.z
         + shIrradiance[8] * 0.546274 * (n.x * n.x - n.y * n.y);
}

// fresnel over the whole hemisphere instead of one H, rough surfaces reflect less at grazing angles
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 calculateAmbient(vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness, float ao)
{
    if (!useIBL) {
        return vec3(ambientStrength) * albedo * ao;
    }

    float NdotV = max(dot(N, V), 0.0);
    vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kD = (1.0 - F) * (1.0 - metallic);

    // diffuse, irradiance from the SH around the normal
    vec3 diffuse = max(evaluateSH(N), vec3(0.0)) * albedo;

    // specular, split sum: prefiltered radiance along R times the BRDF integral
    vec3 R = reflect(-V, N);
    vec3 prefiltered = textureLod(prefilterMap, R, roughness * prefilterMaxLod).rgb;
    vec2 envBRDF = texture(brdfLUT, vec2(NdotV, roughness)).rg;
    vec3 specular = prefiltered * (F0 * envBRDF.x + envBRDF.y);

    return (kD * diffuse + specular) * ao * iblIntensity;
}

void main()
{
    vec4 albedoSample = texture(albedoMap, TexCoords);
    vec3 albedo = pow(albedoSample.rgb, vec3(2.2)) * albedoFactor.rgb;
    float alpha = albedoSample.a * albedoFactor.a;

    // glTF's roughness value is in g channel, while metallic is in b channel
    float metallic = texture(metallicMap, TexCoords).b * metallicFactor;
    float roughness = max(texture(roughnessMap, TexCoords).g * roughnessFactor, 0.04);
    float ao = mix(1.0, texture(aoMap, TexCoords).r, aoStrength);
    vec3 emissive = pow(texture(emissiveMap, TexCoords).rgb, vec3(2.2)) * emissiveFactor;

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
        float distance = length(lightPositions[i] - WorldPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        Lo += calculateLo(N, V, L, radiance, F0, albedo, metallic, roughness);
    }

    vec3 ambient = calculateAmbient(N, V, F0, albedo, metallic, roughness, ao);
    vec3 color   = ambient + Lo + emissive;

    FragColor = vec4(color, alpha);

    if (FragColor.a < 0.01)
        discard;
}
