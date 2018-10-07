
#include "stdafx.h"


#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "ImageHolder.h"



 ImageHolder::ImageHolder(const char* path){
	 loadImage(path);
	 this->m_loaded = true;
	 std::cout << "Constructor"<< std::endl;
}

 ImageHolder::ImageHolder() {
	 this->m_loaded = false;
	 std::cout << "DefaultConstructor"<< std::endl;
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

	return m_width * m_height * m_channels;
}

stbi_uc *ImageHolder::getRaw() {
	
	if (!m_loaded) throw new std::exception("Image not loaded yet!");

	return m_ppixels;
}
