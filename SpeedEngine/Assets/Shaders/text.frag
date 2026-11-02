#version 330 core

in vec2 glyphUV;
uniform sampler2D glyphAtlas;
uniform vec4 textColor;
layout(location = 0) out vec4 fragColor;

void main()
{
	float coverage = texture(glyphAtlas, glyphUV).r;
	fragColor = vec4(textColor.rgb, textColor.a * coverage);
}
