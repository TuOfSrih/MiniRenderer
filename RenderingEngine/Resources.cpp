#include "stdafx.h"
#include "Resources.h"
#include "Settings.h"
#include "Utils.h"

#include <filesystem>
#include <regex>

namespace fs = std::filesystem;

Resources* Resources::active;


void Resources::init() {

	active = new Resources();
}

void Resources::destroy() {

	delete Resources::active;
}

void Resources::releaseTextures() {
	
	Resources::active->textures.clear();
}

Resources::Resources(){

}


VkShaderModule Resources::getShader(std::string name) {
	
	return active->shadersModules[name];
}

MeshHolder* Resources::getMesh(std::string name) {

	return active->meshes[name];
}

ImageHolder* Resources::getTexture(std::string name) {

	return active->textures[name];
}

void Resources::loadAllShaders() {

	//Create path to shader directory
	fs::path curPath = fs::current_path() / Settings::getShaderDir();
	std::regex shaderRegEx("(.*)\\.spv");


	for (auto& entry : fs::directory_iterator(curPath)) {

		if (!entry.is_regular_file()) {

			continue;
		}
		std::string shaderName = entry.path().filename().string();

		//Is Shader
		if (!std::regex_match(shaderName, shaderRegEx)) {
			
			continue;
		}

		//Load the Shader
		VkShaderModule module;
		loadShader(entry.path().filename().string(), &module);
		active->shadersModules.emplace(shaderName, module);
	}

}
