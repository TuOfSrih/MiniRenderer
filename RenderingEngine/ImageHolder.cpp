
#include "stdafx.h"

#include <iostream>

#include "Utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "ImageHolder.h"



 ImageHolder::ImageHolder(const char* path){
	 
	 loadImage(path);
	 this->m_loaded = true;
}

 ImageHolder::ImageHolder() {
	 
	 this->m_loaded = false;
 }


ImageHolder::~ImageHolder(){
	
	destroy();
}



void ImageHolder::loadImage(const char* path) {

	if (m_loaded) throw new std::exception("Image already loaded!");
	
	m_ppixels = stbi_load(path, &m_width, &m_height, &m_channels, STBI_rgb_alpha);
	
	if (!m_ppixels)	throw new std::exception("Could not load Image!");
	m_loaded = true;
 }

void ImageHolder::destroy() {
	
	if (m_loaded) {
		
		stbi_image_free(m_ppixels);
		m_loaded = false;
	}

	if (m_uploaded) {

		vkDestroySampler(m_device, m_sampler, nullptr);
		vkDestroyImageView(m_device, m_imageView, nullptr);
		vkDestroyImage(m_device, m_image, nullptr);
		vkFreeMemory(m_device, m_imageMemory, nullptr);

		m_uploaded = false;
	}
}

void ImageHolder::upload(const VkDevice & device, VkPhysicalDevice physDevice, VkCommandPool commandPool, VkQueue queue){
	
	if (!m_loaded || m_uploaded) {
		throw new std::exception("Image was not loaded or already uploaded");
	}

	this->m_device = device;
	VkDeviceSize imageSize = getSize();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, getRaw(), static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	createImage( getWidth(), getHeight(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_image, m_imageMemory);
	
	changeLayout(device, commandPool, queue, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	writeBufferToImage(device, commandPool, queue, stagingBuffer);
	changeLayout(device, commandPool, queue, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);

	m_imageView = createImageView( m_image, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);

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

	ASSERT_VK(vkCreateSampler(device, &samplerCreateInfo, nullptr, &m_sampler));

	m_uploaded = true;
}

void ImageHolder::writeBufferToImage(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkBuffer buffer) {
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
	bufferImageCopy.imageExtent = { (uint32_t)m_width, (uint32_t)m_height, 1};

	vkCmdCopyBufferToImage(commandBuffer, buffer, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
	
	endOneTimeCommandBuffer( queue, commandPool, commandBuffer);

}

void ImageHolder::changeLayout(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkImageLayout imageLayout) {
	
	changeImageLayout( queue, m_image, VK_FORMAT_R8G8B8A8_UNORM, this->m_imageLayout, imageLayout);

	this->m_imageLayout = imageLayout;
}

int ImageHolder::getWidth() {

	if(!m_loaded) throw new std::exception("Image not loaded yet!");

	return m_width;
}

int ImageHolder::getHeight() {

	if (!m_loaded) throw new std::exception("Image not loaded yet!");

	return m_height;
}

int ImageHolder::getChannels() {
	
	if (!m_loaded) throw new std::exception("Image not loaded yet!");

	return 4;
}

int ImageHolder::getSize() {
	
	if (!m_loaded) throw new std::exception("Image not loaded yet!");

	return m_width * m_height * getChannels();//TODO avoid confusion with channels
}

stbi_uc *ImageHolder::getRaw() {
	
	if (!m_loaded) throw new std::exception("Image not loaded yet!");

	return m_ppixels;
}

VkSampler ImageHolder::getSampler() {
	
	if (!m_loaded) throw new std::exception("Image not loaded yet!");

	return m_sampler;
}

VkImageView ImageHolder::getImageView() {

	if (!m_loaded) throw new std::exception("Image not loaded yet!");

	return m_imageView;

}
