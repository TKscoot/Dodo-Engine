#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 camPos;
} ubo;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inVertPos;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D albedoSampler;
layout(binding = 2) uniform sampler2D normalSampler;
layout(binding = 3) uniform sampler2D metallicSampler;
layout(binding = 4) uniform sampler2D roughnessSampler;

layout(push_constant) uniform PushConsts {
	layout(offset = 12) float roughness;
	layout(offset = 16) float metallic;
	layout(offset = 20) float r;
	layout(offset = 24) float g;
	layout(offset = 28) float b;
} material;

const float PI = 3.14159265359;

vec3 materialcolor()
{
	vec4 diffuseColor = texture(albedoSampler, inTexCoord);
	return /*vec3(material.r, material.g, material.b) + */  diffuseColor.rgb;
}

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(normalSampler, inTexCoord).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(inWorldPos);
    vec3 Q2  = dFdy(inWorldPos);
    vec2 st1 = dFdx(inTexCoord);
    vec2 st2 = dFdy(inTexCoord);

    vec3 N   = normalize(inNormal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
	//vec3 albedo     = materialcolor();
	vec3 albedo     = pow(texture(albedoSampler, inTexCoord).rgb, vec3(2.2));
    float metallic  = texture(metallicSampler,   inTexCoord).r;
    float roughness = texture(roughnessSampler,  inTexCoord).r;
	//metallic = material.metallic;
	//roughness = material.roughness;

    //vec3 N = normalize(inNormal);
    vec3 N = getNormalFromMap();
    vec3 V = normalize(ubo.camPos - inWorldPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);

	//const vec3 lightPos = vec3(15.0,-7.5,15.0);

	vec3 lightPos[4];
	lightPos[0] = vec3(-10.0f,  10.0f, 10.0f);
	lightPos[1] = vec3( 10.0f,  10.0f, 10.0f);
	lightPos[2] = vec3(-10.0f, -10.0f, 10.0f);
	lightPos[3] = vec3( 10.0f, -10.0f, 10.0f);


	vec3 lightColor[4];
	lightColor[0] = vec3(300.0f, 300.0f, 300.0f);
	lightColor[1] = vec3(300.0f, 300.0f, 300.0f);
	lightColor[2] = vec3(300.0f, 300.0f, 300.0f);
	lightColor[3] = vec3(300.0f, 300.0f, 300.0f);

    for(int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        vec3 L			  = normalize(lightPos[i] - inWorldPos);
        vec3 H			  = normalize(V + L);
        float dist		  = length(lightPos[i] - inWorldPos);
        float attenuation = 1.0 / (dist * dist);
        vec3 radiance     = lightColor[i] * attenuation;

        // cook-torrance brdf
        float D = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        //vec3 numerator    = D * G * F;
        //float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
        //vec3 specular     = numerator / max(denominator, 0.001);

		float NdotV = max(dot(N, V), 0.00000001);
		float NdotL = max(dot(N, L), 0.00000001);

		vec3 specular = D * G * F;
		specular /= 4.0 * NdotV * NdotL;

        // add to outgoing radiance Lo
        Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo.rgb  * 1.0;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    //outColor = vec4(color, 1.0);
	outColor = vec4(color, 1.0);
}
