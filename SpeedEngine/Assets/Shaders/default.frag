#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

// Material properties
uniform vec4 color;
uniform bool useTexture;
uniform float intensity;

// Texture atlas properties
uniform int frameIndex;
uniform float numRows;
uniform float numCols;

// Textures
uniform sampler2D diffuse0;
uniform sampler2D diffuse1;

void main()
{
    // Calculate UV for texture atlas
    float col = float(frameIndex % int(numCols));
    float row = float(frameIndex / int(numCols));
    
    vec2 uvScale = vec2(1.0 / numCols, 1.0 / numRows);
    vec2 uvOffset = vec2(col * uvScale.x, (numRows - 1.0 - row) * uvScale.y);
    vec2 finalUV = (TexCoord * uvScale) + uvOffset;
    
    // Sample texture
    vec4 texColor;
    if (useTexture) {
        texColor = texture(diffuse0, finalUV);
    } else {
        texColor = texture(diffuse1, finalUV);
    }
    
    // Simple directional lighting
    vec3 lightDir = normalize(vec3(0.0, 1.0, 0.0));
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, lightDir), 0.0);
    
    // Ambient + diffuse lighting
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vec3(1.0);
    vec3 diffuse = diff * vec3(1.0);
    vec3 lighting = (ambient + diffuse) * intensity;
    
    // Combine texture and color with lighting
    // vec4 finalColor = texColor *color;
    vec4 finalColor = color;
    finalColor.rgb *= lighting;
    
    FragColor = finalColor;
    
    // Discard transparent fragments
    if (FragColor.a < 0.01) {
        discard;
    }
}