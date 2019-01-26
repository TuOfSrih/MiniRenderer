
#pragma once

//#define STB_IMAGE_IMPLEMENTATION
#include "stdafx.h"
#include "stb_image.h"

class ImageHolder {

private:

	VkImage			image;
	VkDeviceMemory	imageMemory;
	VkImageView		imageView;
	VkImageLayout	imageLayout		= VK_IMAGE_LAYOUT_PREINITIALIZED;
	VkSampler		sampler;

	int				width;
	int				height;
	int				amountChannels;
	stbi_uc*		pixels;

	bool			onGPU		= false;


	void loadImage(const char* path);
	
	void changeLayout(VkQueue queue, VkImageLayout imageLayout);
	void writeBufferToImage(VkQueue queue, VkBuffer buffer);

public:

	explicit ImageHolder(const char* path);
	~ImageHolder();
	ImageHolder(const ImageHolder &)			= delete;
	ImageHolder(ImageHolder &&)					= delete;
	ImageHolder& operator=(const ImageHolder &) = delete;
	ImageHolder& operator=(ImageHolder &&)		= delete;

	void		transferToGPU(VkQueue queue);
	void		destroy();

	int			getWidth();
	int			getHeight();
	int			getChannels();
	int			getSize();
	VkSampler	getSampler();
	VkImageView getImageView();
	stbi_uc*	getRaw();
};

