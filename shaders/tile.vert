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

struct TileData
{
	vec4 position;
	uint tileTemplateIndex;
	uint tileVariantIndex;
};

layout(std430, binding = 1) restrict readonly buffer Tiles
{
	TileData in_tiles[];
};

struct TileTemplateData
{
	uint64_t albedoTexture;
	int numVariants;
	int numAnimationFrames;
};

layout(std430, binding = 2) restrict readonly buffer TileTemplates
{
	TileTemplateData in_tileTemplates[];
};

layout (location = 0) in vec3 in_Vertex;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_Uv;

layout (location = 0) out vec3 out_Normal;
layout (location = 1) out vec2 out_Uv;
layout (location = 2) out flat int out_BaseInstance;

void main()
{
	TileData tileData = in_tiles[gl_BaseInstance];
	TileTemplateData tileTemplateData = in_tileTemplates[tileData.tileTemplateIndex];
	
	mat4 mvp = projection * view;
	gl_Position = mvp * vec4(in_Vertex + tileData.position.xyz, 1.0);
	out_Normal = in_Normal;
	out_Uv = vec2(in_Uv.x, in_Uv.y + float(tileData.tileVariantIndex) / tileTemplateData.numVariants);
	out_BaseInstance = gl_BaseInstance;
}
