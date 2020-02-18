#pragma once
#include <glm/glm.hpp>

struct SpotLight
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 direction = glm::vec3(0.0f, 0.0f, 1.0f);
	float innerCutOff = glm::cos(glm::radians(12.5f));
	float outerCutOff = glm::cos(glm::radians(15.0f));

	glm::vec3 ambient = glm::vec3(0.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);

	float constant = 1.0f;
	float linear = 0.09f;
	float quadratic = 0.032f;
};

struct PointLight
{
	glm::vec3 position = glm::vec3(0.0f);

	float constant = 1.0f;
	float linear = 1.0f;
	float quadratic = 1.0f;

	glm::vec3 ambient = glm::vec3(1.0f);
	glm::vec3 diffuse = glm::vec3(1.0f);
	glm::vec3 specular = glm::vec3(1.0f);
};

struct DirectionalLight
{
	glm::vec3 direction = glm::vec3(-0.2f, -1.0f, -0.3f);

	glm::vec3 ambient = glm::vec3(0.05f);
	glm::vec3 diffuse = glm::vec3(0.4f);
	glm::vec3 specular = glm::vec3(0.5f);
};