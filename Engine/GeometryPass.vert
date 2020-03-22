
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT
{
	vec3 FragPos;
	vec2 TexCoords;
	vec3 Normal;
} vs_out;

layout (std140) uniform Matrices 
{
	mat4 projection;
	mat4 view;
};

uniform mat4 model;

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0);
	vs_out.FragPos = worldPos.xyz;
	vs_out.TexCoords = aTexCoords;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vs_out.Normal = normalMatrix * aNormal;

	gl_Position = projection * view * worldPos;
}