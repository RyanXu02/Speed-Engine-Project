#version 330 core

in vec2 glyphUV;
uniform sampler2D glyphAtlas;
uniform vec4 textColor;
uniform int drawMode; // 0: glyph coverage, 1: solid color, 2: RGBA texture
layout(location = 0) out vec4 fragColor;

void main()
{
	vec4 sampled = vec4(1.0);
	if (drawMode == 0)
		sampled.a = texture(glyphAtlas, glyphUV).r;
	else if (drawMode == 2)
		sampled = texture(glyphAtlas, glyphUV);
	fragColor = textColor * sampled;
}
