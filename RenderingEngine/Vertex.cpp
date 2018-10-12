
#include "stdafx.h"

#include "Vertex.h"

#include "Utils.h"





Vertex::~Vertex()
{
}


VkVertexInputBindingDescription Vertex::getVertexInputBindingDescription() {
	
	VkVertexInputBindingDescription vertexInputBindingDescription;
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.stride = sizeof(Vertex);
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return vertexInputBindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getVertexInputAttributDescriptions() {
	
	std::vector<VkVertexInputAttributeDescription> vertexInputAttributDescriptions(3);

	vertexInputAttributDescriptions[0].location = 0;
	vertexInputAttributDescriptions[0].binding = 0;
	vertexInputAttributDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributDescriptions[0].offset = offsetof(Vertex, pos);

	vertexInputAttributDescriptions[1].location = 1;
	vertexInputAttributDescriptions[1].binding = 0;
	vertexInputAttributDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributDescriptions[1].offset = offsetof(Vertex, color);

	vertexInputAttributDescriptions[2].location = 2;
	vertexInputAttributDescriptions[2].binding = 0;
	vertexInputAttributDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributDescriptions[2].offset = offsetof(Vertex, uv);

	return vertexInputAttributDescriptions;
}

bool Vertex::operator==(const Vertex& other) const {

	return pos == other.pos &&
		color == other.color &&
		uv == other.uv;
}

