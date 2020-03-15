#version 330 core 
in vec3 fragNor;
out vec4 color;

uniform vec3 lightIntensity;
uniform vec3 lightDropoff;
uniform sampler2D Texture0;

in vec2 vTexCoord;
in vec3 viewDir;
in vec3 lightDir;
in vec2 cardCoord;

void main()
{
	vec3 raw_color = texture(Texture0, vTexCoord).xyz;
	vec3 scaled_color = .5 * raw_color;
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
	vec3 view = normalize(viewDir);
	float d = length(lightDir);
	float theta = dot(normal, light);
	float phi = dot(normal, normalize(light+view));
	vec3 Ncolor = lightIntensity * (scaled_color + (1/(lightDropoff.x + lightDropoff.y*d + lightDropoff.z*d*d))*(scaled_color * max(0.0,theta) + scaled_color * pow(max(0.0, phi), 1.0)) );
	color = vec4(Ncolor, 1.0);

	if (.20 > raw_color.x && .20 > raw_color.y && .20 > raw_color.z &&
	   (cardCoord.x > 390 || cardCoord.x < -425 || cardCoord.y > 290 || cardCoord.y < -290)) {
		discard;
	}

}
