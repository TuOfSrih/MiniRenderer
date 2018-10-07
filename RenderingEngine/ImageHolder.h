
#pragma once

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "stdafx.h"

class ImageHolder
{
private:
	int m_width;
	int m_height;
	int m_channels;
	stbi_uc *m_ppixels;
	bool m_loaded = false;	//TODO not necessary
	//VkAccessFlags m_image;

public:
	explicit ImageHolder(const char* path);
	explicit ImageHolder();
	~ImageHolder();
	void loadImage(const char* path);
	void destroy();
	int getWidth();
	int getHeight();
	int getChannels();
	int getSize();
	stbi_uc *getRaw();
};

