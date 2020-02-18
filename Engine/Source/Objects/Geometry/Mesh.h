#pragma once
#include <vector>
#include "..\..\Graphics\Texture.h"
#include "..\..\Graphics\Shaders.h"
#include "..\..\Graphics\Vertex.h"

class Mesh
{
public:
	std::vector<Vertex> verticies;
	std::vector<unsigned int> indicies;
	std::vector<Texture> textures;

	Mesh(std::vector<Vertex> verticies, std::vector<unsigned int> indicies, std::vector<Texture> textures);
	void Draw(Shader& shader);
	void Destroy();

private:
	unsigned int VAO, VBO, EBO;
	void SetupMesh();

};