#pragma once

#include "stdafx.h"


class Vertex
{
public:
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
	glm::vec3 normals;

	static VkVertexInputBindingDescription getVertexInputBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getVertexInputAttributDescriptions();
	bool operator==(const Vertex& other) const;

	Vertex(glm::vec3 pos, glm::vec3 color, glm::vec2 uv, glm::vec3 normals)
		: pos(pos), color(color), uv(uv), normals(normals) {};
	~Vertex();

private: 


};

//namespace std {
//
//	template<> struct hash<Vertex> {
//
//		size_t operator()(Vertex const &vert) const {
//			size_t h1 = hash<glm::vec3>()(vert.pos);
//			size_t h2 = hash<glm::vec3>()(vert.color);
//			size_t h3 = hash<glm::vec2>()(vert.uv);
//
//			return ((h1 ^ (h2 << 1)) >> 1) ^ h3;
//		}
//	};
//}

