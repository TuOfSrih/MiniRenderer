
#pragma once

#include "stdafx.h"

#include "Vertex.h"

class MeshHolder {

private:
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;

public:
	MeshHolder();
	~MeshHolder();

	void create(const char* path);

	std::vector<Vertex> getVertices();
	std::vector<uint32_t> getIndices();
};

