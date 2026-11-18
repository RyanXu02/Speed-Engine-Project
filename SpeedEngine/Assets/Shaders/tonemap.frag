#version 330 core

in vec2 texCoord;
layout(location = 0) out vec4 fragColor;

uniform sampler2D hdrColor;   // linear HDR scene color, RGBA16F
uniform float exposure;
uniform float gamma;
uniform bool useDither;
uniform int tonemapCurve;

float interleavedGradientNoise(vec2 p)  // per pixel random between 0 and 1
{
    return fract(52.9829189 * fract(dot(p, vec2(0.06711056, 0.00583715))));
}

// standard simple reinhard tone mapper
vec3 reinhard(vec3 color) {
    return color / (color + vec3(1.0));
}

// Krzysztof Narkowicz's curve fit of the ACES filmic tonemapper, more contrast in the mid tones
vec3 aces(vec3 color) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((color * (a * color + b)) / (color * (c * color + d) + e), 0.0, 1.0);
}

void main()
{
    vec4 hdr = texture(hdrColor, texCoord);
    vec3 color = hdr.rgb * exposure;

    // 0 none, 1 reinhard, 2 aces, must match the defined enum from cpp side
    if (tonemapCurve == 1) {
        color = reinhard(color);
    } else if (tonemapCurve == 2) {
        color = aces(color);
    } else {
        color = clamp(color, 0.0, 1.0); // None, no curve
    }

    color = pow(color, vec3(1.0 / gamma));

    if (useDither) {    // prevent gradients from snapping into visible bands
        color += (interleavedGradientNoise(gl_FragCoord.xy) - 0.5) / 255.0;
    }

    fragColor = vec4(color, hdr.a);
}
