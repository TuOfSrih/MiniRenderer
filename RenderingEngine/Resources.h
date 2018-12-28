#pragma once

#include <unordered_map>
#include <string>

#include "MeshHolder.h"
#include "ImageHolder.h"

class Resources {

private:

	static std::unordered_map<std::string, MeshHolder*> meshes;
	static std::unordered_map<std::string, VkShaderModule> shadersModules;
	static std::unordered_map<std::string, ImageHolder*> textures;

public:

	Resources(std::string path);
	~Resources();

	static VkShaderModule getShader(std::string name);
	static MeshHolder* getMesh(std::string name);
	static ImageHolder* getTexture(std::string name);
};

