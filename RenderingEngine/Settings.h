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

	Settings();
	~Settings();

	std::string& getShaderDir();
};

