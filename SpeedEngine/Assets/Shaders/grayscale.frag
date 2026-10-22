#version 330 core

in vec2 texCoord;
layout(location = 0) out vec4 fragColor;

uniform sampler2D sceneColor;

void main()
{
	vec4 color = texture(sceneColor, texCoord);
	float gray = dot(color.rgb, vec3(0.2162, 0.7152, 0.0722)); //https://en.wikipedia.org/wiki/Grayscale
	fragColor = vec4(vec3(gray), color.a);
}
