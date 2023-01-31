//
#version 460 core

#extension GL_ARB_bindless_texture : require
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

layout (location = 0) in vec3 in_Normal;
layout (location = 1) in vec2 in_Uv;
layout (location = 2) in flat int in_BaseInstance;

layout (location = 0) out vec4 out_FragColor;

float random( vec2 p )
{
    vec2 K1 = vec2(
        23.14069263277926, // e^pi (Gelfond's constant)
         2.665144142690225 // 2^sqrt(2) (Gelfondâ€“Schneider constant)
    );
    return fract( cos( dot(p,K1) ) * 12345.6789 );
}

float remap(float value, float min1, float max1, float min2, float max2)
{
	return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void main()
{
	TileData tileData = in_tiles[in_BaseInstance];
	TileTemplateData tileTemplateData = in_tileTemplates[tileData.tileTemplateIndex];

	// pick grass or dirt color based on normal
	//vec4 textureColor = in_Normal.z > 0.5 ? grassColor : dirtColor;

	vec4 textureColor = vec4(in_Uv, 0.0, 1.0);
	textureColor = texture( sampler2D(unpackUint2x32(tileTemplateData.albedoTexture)), in_Uv);

	// add some randomness to the input color
	//vec4 color = vec4(textureColor.rgb + (random(gl_FragCoord.xy) * 0.1 - 0.05), textureColor.a);
	vec4 color = textureColor;

	// apply shadow
	float dotNormalLightDirection = dot(in_Normal, lightDirection.xyz);
	float shadowFactor = remap(clamp(-dotNormalLightDirection, 0.0, 1.0), 0.0, 1.0, 0.7, 1.0);
	out_FragColor = vec4(color.rgb * shadowFactor, color.a);
};
