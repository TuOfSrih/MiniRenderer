#pragma once

struct Fixed {

	std::string shaderDir;
};

struct Flexible {

	VkDevice			renderingDevice;
	VkPhysicalDevice	physDevice;
};

class Settings {

private:

	Fixed		fixed;
	Flexible	flex;

	static Settings* active;

public:

	static void init();
	static void destroy();
	

	Settings();
	~Settings()									= default;
	Settings(const Settings& other)				= delete;
	Settings(Settings&& other)					= default;
	Settings& operator=(const Settings& other)	= delete;
	Settings& operator=(Settings&& other)		= default;


	static std::string&			getShaderDir();
	static VkDevice&			getDevice();
	static VkPhysicalDevice&	getPhysDevice();

};

