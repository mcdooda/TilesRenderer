//
#version 460 core

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : enable

layout(std140, binding = 0) uniform PerFrameData
{
	mat4 view;
	mat4 projection;
	vec4 color;
	vec4 lightDirection;
};

layout (location = 0) in vec4 in_Color;
layout (location = 1) in vec3 in_Normal;

layout (location = 0) out vec4 out_FragColor;

float remap(float value, float min1, float max1, float min2, float max2)
{
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main()
{
	float dotNormalLightDirection = dot(in_Normal, lightDirection.xyz);
	float shadowFactor = remap(clamp(-dotNormalLightDirection, 0.0, 1.0), 0.0, 1.0, 0.7, 1.0);
	out_FragColor = vec4(in_Color.rgb * shadowFactor, in_Color.a);
};
