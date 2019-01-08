#pragma once

#include <unordered_map>
#include <string>

#include "MeshHolder.h"
#include "ImageHolder.h"

class Resources {

private:

	static Resources* active;

	std::unordered_map<std::string, MeshHolder*>		meshes;
	std::unordered_map<std::string, VkShaderModule>		shadersModules;
	std::unordered_map<std::string, ImageHolder*>		textures;

public:

	static void init();
	static void destroy();
	

	Resources();
	Resources(const Resources&)				= delete;
	Resources(Resources&&)					= default;
	Resources& operator=(const Resources&)	= delete;
	Resources& operator=(Resources&&)		= default;
	~Resources()							= default;

	static MeshHolder*		getMesh(std::string name);
	static VkShaderModule	getShader(std::string name);
	static ImageHolder*		getTexture(std::string name);

	static void				loadAllShaders();

};

