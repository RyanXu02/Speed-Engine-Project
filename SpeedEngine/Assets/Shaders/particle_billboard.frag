#version 330 core

in vec2 uv;
in vec4 particleColor;

out vec4 fragColor;

void main()
{
	// soft round sprite
//	float d = length(uv - vec2(0.5)) * 2.0;
//	float alpha = 1.0 - smoothstep(0.7, 1.0, d);
//	fragColor = vec4(particleColor.rgb, particleColor.a * alpha);
	// solid quad until textures are added
	fragColor = particleColor;
}
