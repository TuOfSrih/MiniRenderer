#include "stdafx.h"

#include "MeshHolder.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"


MeshHolder::MeshHolder() {}


MeshHolder::~MeshHolder() {}

void MeshHolder::create(const char* path) {

	tinyobj::attrib_t vertexAttributes;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errorMsg = "Error while parsing mesh!";

	if (!tinyobj::LoadObj(&vertexAttributes, &shapes, &materials, &errorMsg, path)) {
		
		throw std::exception("Failed to parse mesh");
	}

	for (tinyobj::shape_t shape : shapes) {		//TODO check fourth component, general .obj files
		for (tinyobj::index_t index : shape.mesh.indices) {

			glm::vec3 pos = {vertexAttributes.vertices[3 * index.vertex_index ],
							 vertexAttributes.vertices[3 * index.vertex_index + 2],
							 vertexAttributes.vertices[3 * index.vertex_index + 1]};

			glm::vec2 uv = { 2 * vertexAttributes.texcoords[index.texcoord_index],		//TODO use uvs
							 2 * vertexAttributes.texcoords[index.texcoord_index + 1] };

			glm::vec3 normals = { vertexAttributes.normals[3 * index.normal_index],
				vertexAttributes.normals[3 * index.normal_index + 2],
				vertexAttributes.normals[3 * index.normal_index + 1] };

			Vertex vertex(pos, glm::vec3{ 0.0f, 1.0f, 0.0f }, glm::vec2{ 0.0f, 0.0f }, normals);

			m_vertices.push_back(vertex);
			m_indices.push_back(m_indices.size());//TODO check
		}
	}
}

std::vector<Vertex> MeshHolder::getVertices() {		//TODO avoid copymess

	return m_vertices;
}

std::vector<uint32_t> MeshHolder::getIndices() {

	return m_indices;
}
