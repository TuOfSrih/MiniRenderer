#pragma once

struct Fixed {

	std::string shaderDir;
};

struct Flexible {

};

class Settings {

private:

	Fixed fixed;
	Flexible flex;

public:

	static Settings active;

	Settings();
	//~Settings();
	Settings(const Settings& other) = delete;
	Settings(Settings&& other) = default;
	Settings& operator=(const Settings& other) = delete;
	Settings& operator=(Settings&& other) = default;


	std::string& getShaderDir();
};

