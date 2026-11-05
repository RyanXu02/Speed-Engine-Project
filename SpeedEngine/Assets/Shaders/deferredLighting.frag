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
uniform float penumbraScale;  // UV of blur per unit of depth gap (bigger light = softer)
uniform float searchRadius;   // UV radius of the blocker search
uniform bool usePCSS;
uniform sampler2D blueNoise;  // tiled per pixel to rotate the PCSS samples
uniform bool useBlueNoise;

// point lights
#define MAX_LIGHTS 16
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 camPos;
uniform float G;     // G scattering term
uniform float scatteringScale;

// tunables
uniform float ambientStrength;


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

// different disk version
// const vec2 poissonDisk16[16] = vec2[](
//     vec2(-0.1741, 0.0197), vec2(0.0263, -0.1667),
//     vec2(0.1556, 0.1533), vec2(-0.2520, -0.3392),
//     vec2(0.4209, -0.0654), vec2(-0.1610, 0.4448),
//     vec2(-0.4557, 0.1623), vec2(0.2057, -0.5641),
//     vec2(0.4706, 0.4578), vec2(-0.6861, -0.2227),
//     vec2(0.3204, 0.6955), vec2(0.6874, -0.4045),
//     vec2(-0.3541, -0.7423), vec2(-0.8354, 0.3582),
//     vec2(0.7936, 0.3807), vec2(-0.1172, 0.9525)
// );

const vec2 poissonDisk16[16] = vec2[](
    vec2( -0.94201624, -0.39906216 ),
    vec2( 0.94558609, -0.76890725 ),
    vec2( -0.094184101, -0.92938870 ),
    vec2( 0.34495938, 0.29387760 ),
    vec2( -0.91588581, 0.45771432 ),
    vec2( -0.81544232, -0.87912464 ),
    vec2( -0.38277543, 0.27676845 ),
    vec2( 0.97484398, 0.75648379 ),
    vec2( 0.44323325, -0.97511554 ),
    vec2( 0.53742981, -0.47373420 ),
    vec2( -0.26496911, -0.41893023 ),
    vec2( 0.79197514, 0.19090188 ),
    vec2( -0.24188840, 0.99706507 ),
    vec2( -0.81409955, 0.91437590 ),
    vec2( 0.19984126, 0.78641367 ),
    vec2( 0.14383161, -0.14100790 )
);


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

//PCSS shadow ref https://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
//the paper uses perspective shadow with spot light which give a bit differnt result from orthographic shadow so need some math adjustments
//orthographics have light being parallel, dB goes to infinity so this formular isn't used for orthographics shadow
float penumbraSize(float receiver, float blocker) { // parallel plane estimation
    return (receiver - blocker) / blocker; //TODO: maybe * wLight check this
}

float interleavedGradientNoise(vec2 p)  // per pixel random between 0 and 1
{
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

// average depth of the occluders around this pixel
float findBlockerDepth(vec2 uv, float receiverDepth, mat2 rot, out int blockerCount) {
    float sum = 0.0;
    blockerCount = 0;
    for(int i = 0; i < 16; i++) {
        float d = texture(shadowMap, uv + rot * poissonDisk16[i] * searchRadius).r;
        if(d < receiverDepth) {
            sum += d;
            blockerCount++;
        }
    }
    return blockerCount > 0 ? sum / float(blockerCount) : 1.0;
}

// pcf over a disk of the given radius
float pcfPoisson(vec2 uv, float receiverDepth, float radius, mat2 rot) {
    float lit = 0.0;
    for(int i = 0; i < 16; i++) {
        float d = texture(shadowMap, uv + rot * poissonDisk16[i] * radius).r;
        lit += receiverDepth > d ? 0.0 : 1.0;
    }
    
    return lit / 16.0;
}

float calculateShadow(vec3 worldPos, vec3 N, vec3 L)
{
    float normalOffset = shadowNormalOffset * (1.0 - max(dot(N, L), 0.0));  // prevent self shadowing from grazing sun angle
    vec4 lightSpacePos = lightSpaceMatrix * vec4(worldPos + N * normalOffset, 1.0);
    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
    vec3 projCoordsNDC = projCoords * 0.5 + 0.5;

    // lit when outside the light far range
    if(projCoordsNDC.z > 1.0) {
        return 1.0;
    }

    float bias = max(shadowBias * (1.0 - dot(N, L)), shadowBias * 0.1);
    float currentDepth = projCoordsNDC.z - bias;
    float shadow = 0.0;

    if(usePCSS) {
        // one noise texel per screen pixel, texelFetch so it never gets filtered
        float noise = useBlueNoise
            ? texelFetch(blueNoise, ivec2(gl_FragCoord.xy) % textureSize(blueNoise, 0), 0).r
            : interleavedGradientNoise(gl_FragCoord.xy);
        float angle = noise * 2.0 * PI;
        float c = cos(angle);
        float s = sin(angle);
        mat2 rot = mat2(c, s, -s, c);

        int blockerCount;
        float blockerDepth = findBlockerDepth(projCoordsNDC.xy, currentDepth, rot, blockerCount);
        if(blockerCount == 0) { // nothing is blocking, scene is lit
            return 1.0;
        }

        float penumbra = (currentDepth - blockerDepth) * penumbraScale;
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        penumbra = clamp(penumbra, texelSize.x, searchRadius);
        shadow = pcfPoisson(projCoordsNDC.xy, currentDepth, penumbra, rot);
    } 
    else {
        vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
        const int radius = 3;
        for(int i = -radius; i <= radius; i++) {
            for(int j = -radius; j <= radius; j++) {
                float pcfDepth = texture(shadowMap, projCoordsNDC.xy + vec2(i, j) * texelSize).r;   //closest depth
                shadow += currentDepth > pcfDepth ? 0.0 : 1.0;
            }
        }

        shadow /= float((2 * radius + 1) * (2 * radius + 1));
    }

    return shadow;

}

// image based lighting from the ImageBasedRenderer, falls back to the flat ambient when off
uniform bool useIBL;
uniform vec3 shIrradiance[9];       // stands in for irradianceMap, cosine convolved and divided by PI
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform float iblIntensity;

const float MAX_REFLECTION_LOD = 4.0; // prefilterMap mips - 1

uniform bool useSky;
uniform samplerCube environmentMap;
uniform int skySource;
uniform mat4 skyInverseViewProj;

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

    vec3 R = reflect(-V, N);

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = max(evaluateSH(N), vec3(0.0));
    vec3 diffuse = irradiance * albedo;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part
    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F0 * brdf.x + brdf.y); // LearnOpenGL uses F here? the splitsum from Karis paper uses F0

    return (kD * diffuse + specular) * ao * iblIntensity;
}

vec3 sampleSky()
{
    // world direction through this pixel on the far plane
    vec4 farPoint = skyInverseViewProj * vec4(TexCoords * 2.0 - 1.0, 1.0, 1.0);
    vec3 WorldPos = normalize(farPoint.xyz / farPoint.w);

    vec3 envColor = skySource == 1
        ? max(evaluateSH(WorldPos), vec3(0.0))
        : textureLod(environmentMap, WorldPos, 0.0).rgb;
    return envColor * iblIntensity;
}

const mat4 DITHER_PATTERN = mat4(
    vec4(0.0, 0.5, 0.125, 0.625),
    vec4(0.75, 0.22, 0.875, 0.375),
    vec4(0.1875, 0.6875, 0.0625, 0.5625),
    vec4(0.9375, 0.4375, 0.8125, 0.3125)
);

// Henyey-Greenstein phase function
float mieScattering(float cosTheta, float G) {
	float numerator = (1.0f - G*G);
	float denominator =  (4.0f * PI * pow(1.0f + G * G - 2.0f * G * cosTheta, 1.5f));
	return numerator / denominator;
}

void main()
{
    // the G-buffer clears to 0 and geometry always writes a unit normal, so no normal means nothing was drawn
    vec3 rawNormal = texture(gNormal, TexCoords).xyz;
    if (dot(rawNormal, rawNormal) < 1e-6) {
        FragColor = vec4(useSky ? sampleSky() : vec3(0.0), 1.0);
        return;
    }

    float ao = texture(gMaterial, TexCoords).r;
    float roughness = texture(gMaterial, TexCoords).g;
    float metallic = texture(gMaterial, TexCoords).b;
    vec3 emissive = texture(gEmissive, TexCoords).rgb;

    vec3 WorldPos = texture(gPosition, TexCoords).xyz;
    vec3 albedo = texture(gAlbedo, TexCoords).rgb;
    float alpha = texture(gAlbedo, TexCoords).a;
    
    vec3 N = normalize(rawNormal);
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

    vec3 ambient = calculateAmbient(N, V, F0, albedo, metallic, roughness, ao);
    vec3 finalColor   = ambient + Lo + emissive;

    // volumetric light march
    vec3 viewRay = normalize(WorldPos - camPos);
    float maxDist = length(WorldPos - camPos);
    const int numSteps = 16;
    float stepSize = maxDist / float(numSteps);
    float dither = DITHER_PATTERN[int(gl_FragCoord.x) % 4][int(gl_FragCoord.y) % 4]; //Bayer pattern
    vec2 noiseUV = gl_FragCoord.xy / vec2(textureSize(blueNoise, 0));

    vec3 rayPos = camPos;
    vec3 step = viewRay * stepSize;
    rayPos += step * dither;

    vec3 volume = vec3(0.0);
    vec4 color = vec4(0.0);
    float transparency = 1.0;
    float absorptionCoefficient = 0.15;
    for(int i = 0; i < numSteps; i++) {
        vec4 fragPosLight = lightSpaceMatrix * vec4(rayPos, 1.0);   // where the ray position is at each step
        vec3 rayProjCoordsNDC = fragPosLight.xyz / fragPosLight.w * 0.5 + 0.5;
        float shadowDepth = texture(shadowMap, rayProjCoordsNDC.xy).r;
        
        // if the point in the rayPos closer to the sun than the shadowmap
        // shade and accumulate the volume when not in the shade
        float bias = 0.0005;
        if(shadowDepth > rayProjCoordsNDC.z + bias) {   
            float phase = mieScattering(dot(viewRay, -dirLightDirection), G);
            vec3 scattering = dirLightColor * phase * absorptionCoefficient;
            volume += scattering * transparency * stepSize;
        }
        transparency *= exp(-absorptionCoefficient * stepSize);

        if(transparency <= 0.01) {
            break;
        }
        rayPos += step;
    }

    vec3 finalVolume = (volume / float(numSteps)) * scatteringScale;
    finalColor += finalVolume;
    
    FragColor = vec4(finalColor, 1.0);
}
