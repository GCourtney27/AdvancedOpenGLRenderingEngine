#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out VS_OUT
{
	vec3 FragPos;  // Position in world space
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
   vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
   vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
   vs_out.TexCoords = aTexCoords;

   gl_Position = projection * view * vec4(vs_out.FragPos, 1.0);
}