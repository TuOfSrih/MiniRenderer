
#pragma once

//#define STB_IMAGE_IMPLEMENTATION
#include "stdafx.h"
#include "stb_image.h"



class ImageHolder
{
private:
	int m_width;
	int m_height;
	int m_channels;
	stbi_uc *m_ppixels;
	bool m_loaded = false;	//TODO not necessary
	bool m_uploaded = false;
	VkImage m_image;
	VkDeviceMemory m_imageMemory;
	VkImageView m_imageView;
	VkImageLayout m_imageLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	VkDevice m_device;
	VkSampler m_sampler;
	void changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout imageLayout);
	void writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer);

public:
	explicit ImageHolder(const char* path);
	explicit ImageHolder();
	~ImageHolder();
	ImageHolder(const ImageHolder &) = delete;
	ImageHolder(ImageHolder &&) = delete;
	ImageHolder& operator=(const ImageHolder &) = delete;
	ImageHolder& operator=(ImageHolder &&) = delete;


	void loadImage(const char* path);
	void destroy();
	void upload(const VkDevice& device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue);
	int getWidth();
	int getHeight();
	int getChannels();
	int getSize();
	VkSampler getSampler();
	VkImageView getImageView();
	stbi_uc *getRaw();
};

