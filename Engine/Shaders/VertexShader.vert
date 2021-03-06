#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

#define NUM_POINT_LIGHTS 1

out VS_OUT
{
	vec3 FragPos;  // Position in world space
	vec2 TexCoords;
	vec4 FragPosLightSpace;
	vec3 TangentViewPos;
	vec3 TangentFragPos;
	mat3 TBN;
} vs_out;

layout (std140) uniform Matrices 
{
	mat4 projection;
	mat4 view;
};

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

uniform vec3 viewPos;

void main()
{
   vs_out.FragPos = vec3(model * vec4(aPos, 1.0));
   vs_out.TexCoords = aTexCoords;
   vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(vs_out.FragPos, 1.0);

  vec3 T = normalize(vec3(model * vec4(aTangent,   0.0)));
  vec3 N = normalize(vec3(model * vec4(aNormal,    0.0)));
  vec3 B = cross(N, T);
  mat3 TBN = transpose(mat3(T, B, N));

  vs_out.TBN = TBN;
  vs_out.TangentViewPos = TBN * viewPos;
  vs_out.TangentFragPos = TBN * vs_out.FragPos;

   gl_Position = projection * view * model * vec4(aPos, 1.0);
}