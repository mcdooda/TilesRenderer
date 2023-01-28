//
#version 460 core

#extension GL_ARB_gpu_shader_int64 : enable

layout(std140, binding = 0) uniform PerFrameData
{
	mat4 view;
	mat4 projection;
	vec4 grassColor;
	vec4 dirtColor;
	vec4 lightDirection;
};

layout(std430, binding = 1) restrict readonly buffer Tiles
{
	vec4 in_tilePositions[];
};

layout (location = 0) in vec3 in_Vertex;
layout (location = 1) in vec3 in_Normal;

layout (location = 0) out vec3 out_Normal;

void main()
{
	vec4 tilePosition = in_tilePositions[gl_BaseInstance];
	mat4 mvp = projection * view;
	gl_Position = mvp * vec4(in_Vertex + tilePosition.xyz, 1.0);
	out_Normal = in_Normal;
}
