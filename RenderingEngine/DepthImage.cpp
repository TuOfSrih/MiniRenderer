#include "stdafx.h"
#include "DepthImage.h"


DepthImage::DepthImage(){

}


DepthImage::~DepthImage(){

	destroy();
}


void DepthImage::create(VkDevice device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue, uint32_t width, uint32_t height) {

	if (mCreated) throw new std::exception("Depth Image was already created!");

	mDevice = device;

	VkFormat depthFormat = findDepthFormat(physDevice);
	createImage(device, physDevice, width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, mImage, mImageMemory);
	createImageView(device, mImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, mImageView);
	changeImageLayout(device, commandPool, queue, mImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	mCreated = true;
}

void DepthImage::destroy() {

	if (mCreated) {

		vkDestroyImageView(mDevice, mImageView, nullptr);
		vkDestroyImage(mDevice, mImage, nullptr);
		
		vkFreeMemory(mDevice, mImageMemory, nullptr);
		mCreated = false;

		mImage = VK_NULL_HANDLE;		//TODO improve
		mImageView = VK_NULL_HANDLE;
		mImageMemory = VK_NULL_HANDLE,
		mDevice = VK_NULL_HANDLE;
	}
}

VkFormat DepthImage::findDepthFormat(VkPhysicalDevice physDevice) {

	std::vector<VkFormat> possibleFormats = { VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT};

	return findSupportedFormat(physDevice, possibleFormats, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkAttachmentDescription DepthImage::getDepthAttachmentDescription(VkPhysicalDevice physDevice) {

	VkAttachmentDescription attachmentDescription;
	attachmentDescription.flags = 0;
	attachmentDescription.format = findDepthFormat(physDevice);
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

VkImageView DepthImage::getImageView() {

	return mImageView;
}
