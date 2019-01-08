#pragma once

#include <unordered_map>
#include <string>

#include "MeshHolder.h"
#include "ImageHolder.h"

class Resources {

private:


	std::unordered_map<std::string, MeshHolder*> meshes;
	std::unordered_map<std::string, VkShaderModule> shadersModules;
	std::unordered_map<std::string, ImageHolder*> textures;

public:

	static Resources active;

	Resources(std::string path);
	Resources(const Resources&) = delete;
	Resources(Resources&&) = default;
	Resources& operator=(const Resources&) = delete;
	Resources& operator=(Resources&&) = default;
	~Resources();

	MeshHolder*		getMesh(std::string name);
	VkShaderModule	getShader(std::string name);
	ImageHolder*	getTexture(std::string name);
};

