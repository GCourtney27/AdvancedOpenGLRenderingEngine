#include "Shaders.h"

Shader::Shader(const char * vertexPath, const char * fragmentPath, const char * geometryPath)
{
	LoadAndCompile(vertexPath, fragmentPath, geometryPath);

}

void Shader::Init(const char * vertexPath, const char * fragmentPath, const char * geometryPath)
{
	LoadAndCompile(vertexPath, fragmentPath, geometryPath);
}

void Shader::LoadAndCompile(const char * vertexPath, const char * fragmentPath, const char * geometryPath)
{
	// 1. Retrieve the vertex/fragment information source code from filepath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vertShaderFile;
	std::ifstream fragShaderFile;
	std::ifstream geomShaderFile;

	// Ensure ifstream objects can throw exceptions
	vertShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fragShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	geomShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		// Open files
		vertShaderFile.open(vertexPath);
		fragShaderFile.open(fragmentPath);
		std::stringstream vertShaderStream, fragShaderStream, geometryShaderStream;
		// Read file's buffer contents into streams
		vertShaderStream << vertShaderFile.rdbuf();
		fragShaderStream << fragShaderFile.rdbuf();
		// Close file handlers
		vertShaderFile.close();
		fragShaderFile.close();
		// Convert stream into string
		vertexCode = vertShaderStream.str();
		fragmentCode = fragShaderStream.str();

		if (geometryPath != "")
		{
			geomShaderFile.open(geometryPath);
			geometryShaderStream << geomShaderFile.rdbuf();
			geomShaderFile.close();
			geometryCode = geometryShaderStream.str();
		}
	}
	catch (std::ifstream::failure e)
	{
		std::cout << "Error::Shader::Failed to read shader from file: " << e.what() << "\n";

	}
	const char * vertShaderCode = vertexCode.c_str();
	const char * fragShaderCode = fragmentCode.c_str();
	const char * geomShaderCode = geometryCode.c_str();

	// 2. Compile shaders
	unsigned int vertex, fragment, geometry;
	int success;
	char infoLog[512];

	// Vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertShaderCode, NULL);
	glCompileShader(vertex);
	glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex, 512, NULL, infoLog);
		std::cout << "Error::Shader::Vertex::Failed to compile vertex shader\n" << infoLog << "\n";
	}
	// Fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragShaderCode, NULL);
	glCompileShader(fragment);
	glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment, 512, NULL, infoLog);
		std::cout << "Error::Shader::Fragment::Failed to compile fragment shader\n" << infoLog << "\n";
	}
	// Geometry shader
	if (geometryPath != "")
	{
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &geomShaderCode, NULL);
		glCompileShader(geometry);
		glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(geometry, 512, NULL, infoLog);
			std::cout << "Error::Shader::Geometry::Failed to compile geometry shader\n" << infoLog << "\n";
		}
	}

	// 3. Create shader program
	ProgramID = glCreateProgram();
	glAttachShader(ProgramID, vertex);
	glAttachShader(ProgramID, fragment);
	if (geometryPath != "")
		glAttachShader(ProgramID, geometry);
	glLinkProgram(ProgramID);
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(ProgramID, 512, NULL, infoLog);
		std::cout << "Error::Shader::Program::Failed to link shaders\n" << infoLog << "\n";
	}

	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != "")
		glDeleteShader(geometry);
}

Shader::~Shader()
{
	glDeleteProgram(ProgramID);
}

void Shader::Use()
{
	glUseProgram(ProgramID);
}

void Shader::SetBool(const std::string & name, bool value) const
{
	glUniform1i(glGetUniformLocation(ProgramID, name.c_str()), static_cast<int>(value));
}

void Shader::SetInt(const std::string & name, int value) const
{
	glUniform1i(glGetUniformLocation(ProgramID, name.c_str()), value);
}

void Shader::SetFloat(const std::string & name, float value) const
{
	glUniform1f(glGetUniformLocation(ProgramID, name.c_str()), value);
}

void Shader::SetMat4(const std::string & name, glm::mat4 value) const 
{
	glUniformMatrix4fv(glGetUniformLocation(ProgramID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetVec3(const std::string & name, float x, float y, float z) const
{
	glUniform3f(glGetUniformLocation(ProgramID, name.c_str()), x, y, z);
}

void Shader::SetVec3(const std::string & name, glm::vec3 value) const
{
	glUniform3fv(glGetUniformLocation(ProgramID, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetPointLight(const std::string & name, PointLight light)
{
	this->SetVec3(name + ".position", light.position);
	this->SetFloat(name + ".constant", light.constant);
	this->SetFloat(name + ".linear", light.linear);
	this->SetFloat(name + ".quadtratic", light.quadratic);
	this->SetVec3(name + ".ambient", light.ambient);
	this->SetVec3(name + ".diffuse", light.diffuse);
	this->SetVec3(name + ".specular", light.specular);
}

void Shader::SetDirectionalLight(const std::string & name, DirectionalLight light)
{
	this->SetVec3(name + ".direction", light.direction);
	this->SetVec3(name + ".ambient", light.ambient);
	this->SetVec3(name + ".diffuse", light.diffuse);
	this->SetVec3(name + ".specular", light.specular);
}

void Shader::SetSpotLight(const std::string & name, SpotLight light)
{
	this->SetVec3(name + ".position", light.position);
	this->SetVec3(name + ".direction", light.direction);
	this->SetFloat(name + ".innerCutOff", light.innerCutOff);
	this->SetFloat(name + ".outerCutOff", light.outerCutOff);
	this->SetVec3(name + ".ambient", light.ambient);
	this->SetVec3(name + ".diffuse", light.diffuse);
	this->SetVec3(name + ".specular", light.specular);
	this->SetFloat(name + ".constant", light.constant);
	this->SetFloat(name + ".linear", light.linear);
	this->SetFloat(name + ".quadratic", light.quadratic);
}