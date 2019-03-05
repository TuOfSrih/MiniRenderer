#pragma once

#include "stdafx.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "gtx/hash.hpp"

class Vertex {

public:

	Vertex(const glm::vec3& pos, const glm::vec3& color, const glm::vec2& uv, const glm::vec3& normals)
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

namespace std {

	template<> struct hash<Vertex> {

		size_t operator()(Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.uv) << 1);
		}
	};
}
