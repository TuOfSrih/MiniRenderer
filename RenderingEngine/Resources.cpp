#include "stdafx.h"
#include "Resources.h"

#include <filesystem>
#include <regex>

namespace fs = std::filesystem;


Resources::Resources(std::string path){

	//Create path to shader directory
	fs::path curPath = fs::current_path() / path;
	std::regex shaderRegEx(".*\\.(?:vert|frag|tesc|tese|geom|comp)");

	for (auto& entry : fs::directory_iterator(curPath)) {

		if (!entry.is_regular_file()) {

			continue;
		}
		std::string shaderName = entry.path().filename().string();

		//Is Shader
		if (!std::regex_match(shaderName, shaderRegEx)) {
			throw std::exception("Non shader loaded as shader");
		}

		//readFile(path);
		//VkShaderModule shaderModule;
		//createShaderModule();
		//shadersModules.emplace(name, shaderModule);
	}
}


Resources::~Resources()
{
}

VkShaderModule Resources::getShader(std::string name) {

	return shadersModules[name];
}

MeshHolder* Resources::getMesh(std::string name) {

	return meshes[name];
}

ImageHolder* Resources::getTexture(std::string name) {

	return textures[name];
}
