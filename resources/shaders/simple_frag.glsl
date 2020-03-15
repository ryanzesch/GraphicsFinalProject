#version 330 core 
in vec3 fragNor;
out vec4 color;

uniform vec3 MatDif;
uniform vec3 MatAmb;
uniform vec3 MatSpec;
uniform float shine;
uniform vec3 lightIntensity;
uniform vec3 lightDropoff;
in vec3 viewDir;
in vec3 lightDir;
void main()
{
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
	vec3 view = normalize(viewDir);
	float d = length(lightDir);
	float theta = dot(normal, light);
	float phi = dot(normal, normalize(light+view));
	vec3 Ncolor = lightIntensity * (MatAmb + (1/(lightDropoff.x + lightDropoff.y*d + lightDropoff.z*d*d))*(MatDif * max(0.0,theta) + MatSpec * pow(max(0.0, phi), shine)) );
	color = vec4(Ncolor, 1.0);

}
