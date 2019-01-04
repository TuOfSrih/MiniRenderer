
#pragma once
#include "stdafx.h"

#include "Utils.h"


class DepthImage {

private:

	VkImage			mImage			= VK_NULL_HANDLE ;		//unnecessary
	VkDeviceMemory	mImageMemory;
	VkImageView		mImageView		= VK_NULL_HANDLE;
	VkDevice		mDevice			= VK_NULL_HANDLE;
	bool			mCreated		= false;

public:

	DepthImage();
	~DepthImage();
	DepthImage(const DepthImage&) = delete;
	DepthImage(DepthImage&&) = delete;
	DepthImage& operator= (const DepthImage&) = delete;
	DepthImage& operator= (DepthImage&&) = delete;

	void create(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue, uint32_t width, uint32_t height);
	void destroy();

	static VkFormat findDepthFormat(VkPhysicalDevice physDevice);
	static VkAttachmentDescription getDepthAttachmentDescription(VkPhysicalDevice physDevice);
	static VkPipelineDepthStencilStateCreateInfo getPipelineDepthStencilStateCreateInfoOpaque();
	VkImageView getImageView();
};

