//
#version 460 core

layout(std140, binding = 0) uniform PerFrameData
{
	mat4 view;
	mat4 projection;
};

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Color;

layout (location = 0) out vec3 out_Color;

void main()
{
	mat4 mvp = projection * view;
	gl_Position = mvp * vec4(in_Position, 1.0);
	out_Color = in_Color;
}
