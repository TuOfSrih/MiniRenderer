
#pragma once
#include "stdafx.h"



class DepthImage {

private:

	VkImage			image						= VK_NULL_HANDLE ;		//unnecessary
	VkDeviceMemory	imageMemory;
	

	static VkFormat findDepthFormat();

public:

	VkImageView		imageView					= VK_NULL_HANDLE;

	DepthImage(const VkQueue queue);
	~DepthImage();
	DepthImage(const DepthImage&)				= delete;
	DepthImage(DepthImage&&)					= delete;
	DepthImage& operator= (const DepthImage&)	= delete;
	DepthImage& operator= (DepthImage&&)		= delete;

	
	static VkAttachmentDescription					getDepthAttachmentDescription();
	static VkPipelineDepthStencilStateCreateInfo	getPipelineDepthStencilStateCreateInfoOpaque();
};

