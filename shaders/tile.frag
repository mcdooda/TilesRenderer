//
#version 460 core

#extension GL_ARB_bindless_texture : require
#extension GL_ARB_gpu_shader_int64 : enable

layout (location = 0) in vec4 in_Color;
layout (location = 1) in vec3 in_Normal;

layout (location = 0) out vec4 out_FragColor;

void main()
{
	out_FragColor = in_Color;
};
