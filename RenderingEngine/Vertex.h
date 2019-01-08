#pragma once

#include "stdafx.h"

class Vertex {

public:

	Vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uv, glm::vec3 normals)
		: pos(pos), color(color), uv(uv), normals(normals) {};
	~Vertex();

	bool operator==(const Vertex& other) const;

	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
	glm::vec3 normals;

	static VkVertexInputBindingDescription getVertexInputBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributDescriptions();

private: 

};


