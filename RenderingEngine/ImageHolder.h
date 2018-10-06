
#pragma once

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

class ImageHolder
{
private:
	//static ImageHolder* instance;
	int m_width;
	int m_height;
	int m_channels;
	stbi_uc *m_ppixels;
	bool m_loaded = false;	//TODO not necessary

public:
	explicit ImageHolder(const char* path);
	explicit ImageHolder();
	~ImageHolder();
	//static ImageHolder* GetInstance();
	void loadImage(const char* path);
	void destroy();
	int getWidth();
	int getHeight();
	int getChannels();
	int getSize();
	stbi_uc *getRaw();
};

