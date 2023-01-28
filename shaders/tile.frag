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

layout (location = 0) in vec3 in_Normal;

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
	// pick grass or dirt color based on normal
	vec4 textureColor = in_Normal.z > 0.5 ? grassColor : dirtColor;

	// add some randomness to the input color
	vec4 color = vec4(textureColor.rgb + (random(gl_FragCoord.xy) * 0.1 - 0.05), textureColor.a);

	// apply shadow
	float dotNormalLightDirection = dot(in_Normal, lightDirection.xyz);
	float shadowFactor = remap(clamp(-dotNormalLightDirection, 0.0, 1.0), 0.0, 1.0, 0.7, 1.0);
	out_FragColor = vec4(color.rgb * shadowFactor, color.a);
};
