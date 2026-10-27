#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

// Material properties
uniform vec4 color;
uniform float intensity;
uniform vec3 emissiveFactor;
uniform float roughnessFactor;
uniform float metallicFactor;
uniform float aoStrength;
uniform float normalScale;

uniform vec3 viewPos;

// Texture atlas properties
uniform int frameIndex;
uniform float numRows;
uniform float numCols;

// Textures
uniform sampler2D albedo;
uniform sampler2D normalmap;
uniform sampler2D ambientOcc;
uniform sampler2D roughness;
uniform sampler2D emissive;
uniform sampler2D metallic;

uniform bool has_albedo;
uniform bool has_normalmap;
uniform bool has_ambientOcc;
uniform bool has_roughness;
uniform bool has_emissive;
uniform bool has_metallic;

const float PI = 3.14159265359;

vec3 toLinear(vec3 c)
{
    return pow(c, vec3(2.2));
}

// Normal mapping without vertex tangents (cotangent frame from screen-space derivatives)
vec3 perturbNormal(vec3 N, vec3 P, vec2 uv)
{
    vec3 tangentNormal = texture(normalmap, uv).xyz * 2.0 - 1.0;
    tangentNormal.xy *= normalScale;

    vec3 dp1 = dFdx(P);
    vec3 dp2 = dFdy(P);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    mat3 TBN = mat3(T * invmax, B * invmax, N);
    return normalize(TBN * tangentNormal);
}

float distributionGGX(float NdotH, float rough)
{
    float a = rough * rough;
    float a2 = a * a;
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float geometrySmith(float NdotV, float NdotL, float rough)
{
    float k = (rough + 1.0) * (rough + 1.0) / 8.0;
    float gv = NdotV / (NdotV * (1.0 - k) + k);
    float gl = NdotL / (NdotL * (1.0 - k) + k);
    return gv * gl;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // Calculate UV for texture atlas
    float col = float(frameIndex % int(numCols));
    float row = float(frameIndex / int(numCols));

    vec2 uvScale = vec2(1.0 / numCols, 1.0 / numRows);
    vec2 uvOffset = vec2(col * uvScale.x, (numRows - 1.0 - row) * uvScale.y);
    vec2 finalUV = (TexCoord * uvScale) + uvOffset;

    // Sample material
    vec4 baseColor = color;
    if (has_albedo) {
        vec4 texColor = texture(albedo, finalUV);
        baseColor *= vec4(toLinear(texColor.rgb), texColor.a);
    }

    // glTF metallicRoughness: roughness in G, metallic in B
    float rough = roughnessFactor;
    if (has_roughness) {
        rough *= texture(roughness, finalUV).g;
    }
    rough = clamp(rough, 0.04, 1.0);

    float metal = metallicFactor;
    if (has_metallic) {
        metal *= texture(metallic, finalUV).b;
    }

    float ao = 1.0;
    if (has_ambientOcc) {
        ao = mix(1.0, texture(ambientOcc, finalUV).r, aoStrength);
    }

    vec3 emission = emissiveFactor;
    if (has_emissive) {
        emission *= toLinear(texture(emissive, finalUV).rgb);
    }

    vec3 N = normalize(Normal);
    if (has_normalmap) {
        N = perturbNormal(N, FragPos, finalUV);
    }

    // Single directional light, Cook-Torrance BRDF
    vec3 V = normalize(viewPos - FragPos);
    vec3 L = normalize(vec3(0.3, 1.0, 0.5));
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 1e-4);
    float NdotH = max(dot(N, H), 0.0);

    vec3 F0 = mix(vec3(0.04), baseColor.rgb, metal);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float D = distributionGGX(NdotH, rough);
    float G = geometrySmith(NdotV, NdotL, rough);

    vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
    vec3 kD = (vec3(1.0) - F) * (1.0 - metal);
    vec3 lightColor = vec3(3.0) * intensity;
    vec3 direct = (kD * baseColor.rgb / PI + specular) * lightColor * NdotL;

    vec3 ambient = vec3(0.3) * baseColor.rgb * ao;

    vec3 finalColor = ambient + direct + emission;

    // Tonemap and gamma correct
    finalColor = finalColor / (finalColor + vec3(1.0));
    finalColor = pow(finalColor, vec3(1.0 / 2.2));

    FragColor = vec4(finalColor, baseColor.a);

    // Discard transparent fragments
    if (FragColor.a < 0.01) {
        discard;
    }
}
