
#include "stdafx.h"

#include <iostream>

#include "Utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ImageHolder.h"
#include "Settings.h"



 ImageHolder::ImageHolder(const char* path){
	 
	 loadImage(path);
}

ImageHolder::~ImageHolder(){
	
	destroy();
}


void ImageHolder::loadImage(const char* path) {
	
	pixels = stbi_load(path, &width, &height, &amountChannels, STBI_rgb_alpha);
	
	if (!pixels) throw new std::exception("Could not load Image!");
 }

void ImageHolder::destroy() {

	if (onGPU) {

		vkDestroySampler(Settings::getDevice(), sampler, nullptr);
		vkDestroyImageView(Settings::getDevice(), imageView, nullptr);
		vkDestroyImage(Settings::getDevice(), image, nullptr);
		vkFreeMemory(Settings::getDevice(), imageMemory, nullptr);

	}
}

void ImageHolder::transferToGPU(VkQueue queue){
	
	if (onGPU) {
		
		throw new std::exception("Image already on the GPU!");
	}

	VkDeviceSize imageSize = getSize();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory);

	void* data;
	vkMapMemory(Settings::getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, getRaw(), static_cast<size_t>(imageSize));
	vkUnmapMemory(Settings::getDevice(), stagingBufferMemory);

	createImage( getWidth(), getHeight(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
	
	changeLayout(queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	writeBufferToImage( queue, stagingBuffer);
	changeLayout(queue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(Settings::getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(Settings::getDevice(), stagingBufferMemory, nullptr);

	imageView = createImageView( image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

	VkSamplerCreateInfo samplerCreateInfo;
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.pNext = nullptr;
	samplerCreateInfo.flags = 0;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;
	samplerCreateInfo.maxAnisotropy = 16;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;

	ASSERT_VK(vkCreateSampler(Settings::getDevice(), &samplerCreateInfo, nullptr, &sampler));

	onGPU = true;
}

void ImageHolder::writeBufferToImage(VkQueue queue, VkBuffer buffer) {
	
	VkCommandBuffer commandBuffer = startOneTimeCommandBuffer();

	VkBufferImageCopy bufferImageCopy;
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferImageCopy.imageSubresource.mipLevel = 0;
	bufferImageCopy.imageSubresource.baseArrayLayer = 0;
	bufferImageCopy.imageSubresource.layerCount = 1;
	bufferImageCopy.imageOffset = { 0, 0, 0 };
	bufferImageCopy.imageExtent = { (uint32_t)width, (uint32_t)height, 1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
	
	endOneTimeCommandBuffer(queue, Settings::getCommandPool(), commandBuffer);

}

void ImageHolder::changeLayout(VkQueue queue, VkImageLayout dstImageLayout) {
	
	changeImageLayout( queue, image, VK_FORMAT_R8G8B8A8_UNORM, imageLayout, dstImageLayout);

	imageLayout = dstImageLayout;
}

int ImageHolder::getWidth() {

	return width;
}

int ImageHolder::getHeight() {

	return height;
}

int ImageHolder::getChannels() {

	return amountChannels;
}

int ImageHolder::getSize() {

	return width * height * getChannels();
}

stbi_uc *ImageHolder::getRaw() {

	return pixels;
}

VkSampler ImageHolder::getSampler() {

	return sampler;
}

VkImageView ImageHolder::getImageView() {

	return imageView;

}
