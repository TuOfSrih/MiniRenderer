
#pragma once

#include "stdafx.h"

#include "Vertex.h"

class MeshHolder {

private:
	std::vector<Vertex> m_vertices;
	std::vector<uint32_t> m_indices;

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;

	void create(const char* path);

public:
	MeshHolder(const std::string& path);
	MeshHolder(const MeshHolder&) = delete;
	MeshHolder(MeshHolder&&) = default;
	MeshHolder& operator=(const MeshHolder&) = delete;
	MeshHolder& operator=(MeshHolder&&) = default;
	~MeshHolder();

	

	std::vector<Vertex> getVertices();
	std::vector<uint32_t> getIndices();
};

