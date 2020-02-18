#pragma once
#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "..\Objects\Lights\Lights.h"

class Shader
{
public:
	unsigned int ProgramID;

	// Constructor reads and builds the shader
	Shader(const char * vertexPath, const char * fragmentPath);

	// Use/Activate the shader
	void Use();

	// Utility Functions
	void SetBool(const std::string & name, bool value) const;
	void SetInt(const std::string & name, int value) const;
	void SetFloat(const std::string & name, float value) const;
	void SetMat4(const std::string & name, glm::mat4 value) const;
	void SetVec3(const std::string & name, float x, float y, float z) const;
	void SetVec3(const std::string & name, glm::vec3 value) const;

	void SetPointLight(const std::string & name, PointLight light);
	void SetDirectionalLight(const std::string & name, DirectionalLight light);
	void SetSpotLight(const std::string & name, SpotLight light);
};

#endif