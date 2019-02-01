#pragma once

#include "stdafx.h"

#include "Vertex.h"

class MeshHolder {

private:
	std::vector<Vertex>		vertices;
	std::vector<uint32_t>	indices;

	//VkBuffer				vertexBuffer;
	//VkDeviceMemory			vertexMemory;

	void create(const char* path);

public:
	MeshHolder(const std::string& path);
	MeshHolder(const MeshHolder&)				= delete;
	MeshHolder(MeshHolder&&)					= default;
	MeshHolder& operator=(const MeshHolder&)	= delete;
	MeshHolder& operator=(MeshHolder&&)			= default;
	~MeshHolder();

	std::vector<Vertex>		getVertices();
	std::vector<uint32_t>	getIndices();
};

