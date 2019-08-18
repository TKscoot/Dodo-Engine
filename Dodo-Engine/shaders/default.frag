#version 450
#extension GL_ARB_separate_shader_objects : enable

//layout(location = 0) in vec3 fragColor;
//layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) in vec3 fragNormalInterp;
layout(location = 1) in vec3 fragVertPos;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

const vec3 lightPos = vec3(1.0,1.0,1.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float lightPower = 20.0;
const vec3 ambientColor = vec3(0.0, 0.0, 0.0);
const vec3 specColor = vec3(1.0, 1.0, 1.0);
const float shininess = 16.0;
const float screenGamma = 2.2; // Assume the monitor is calibrated to the sRGB color space

void main()
{
	vec4 diffuseColor = texture(texSampler, fragTexCoord);

	vec3 normal = normalize(fragNormalInterp);
	vec3 lightDir = lightPos - fragVertPos;
	float dist = length(lightDir);
	dist = dist * dist;
	lightDir = normalize(lightDir);

	float lambertian = max(dot(lightDir, normal), 0.0);
	float specular = 0.0;

	if(lambertian > 0.0)
	{
		vec3 viewDir = normalize(-fragVertPos);

		vec3 halfDir = normalize(lightDir + viewDir);
		float specAngle = max(dot(halfDir, normal), 0.0);
		specular = pow(specAngle, shininess);
	}

	vec3 colorLinear = ambientColor + diffuseColor.rgb * lambertian * lightColor * lightPower / dist + 
					   specColor * specular * lightColor * lightPower / dist;

	//vec3 colorGammaCorrected = pow(colorLinear, vec3(1.0/screenGamma));
	vec3 colorGammaCorrected = pow(colorLinear, vec3(0.4545));

	outColor = vec4(colorGammaCorrected, 1.0);
}