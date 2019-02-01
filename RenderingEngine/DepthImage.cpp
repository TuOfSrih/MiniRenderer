#include "stdafx.h"

#include "DepthImage.h"
#include "Settings.h"

#include "Utils.h"

//TODO Find out if possible without queue
DepthImage::DepthImage(const VkQueue queue) {

	VkFormat depthFormat = findDepthFormat();

	createImage(Settings::getSwapchainExtent().width, Settings::getSwapchainExtent().height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
	imageView = createImageView(image, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	changeImageLayout(queue, image, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

DepthImage::~DepthImage(){

	vkDestroyImageView(Settings::getDevice(), imageView, nullptr);
	vkDestroyImage(Settings::getDevice(), image, nullptr);

	vkFreeMemory(Settings::getDevice(), imageMemory, nullptr);
}

VkFormat DepthImage::findDepthFormat() {

	std::vector<VkFormat> possibleFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT};

	return findSupportedFormat( possibleFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkAttachmentDescription DepthImage::getDepthAttachmentDescription() {

	VkAttachmentDescription attachmentDescription;
	attachmentDescription.flags = 0;
	attachmentDescription.format = findDepthFormat();
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	return attachmentDescription;
}

VkPipelineDepthStencilStateCreateInfo DepthImage::getPipelineDepthStencilStateCreateInfoOpaque() {

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo;	//Optimize	
	depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCreateInfo.pNext = nullptr;
	depthStencilStateCreateInfo.flags = 0;
	depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.front = {};
	depthStencilStateCreateInfo.back = {};
	depthStencilStateCreateInfo.minDepthBounds = 0.0f;
	depthStencilStateCreateInfo.maxDepthBounds = 1.0f;

	return depthStencilStateCreateInfo;
}

