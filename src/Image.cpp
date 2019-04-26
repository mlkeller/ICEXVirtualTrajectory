#include "Image.hpp"
#include <iostream>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

Image::Image(void* const data, int w, int h, int channels, size_t component_size) : width(w), height(h), channels(channels), component_size(component_size) {
  this->data = new uint8_t[w*h*channels*component_size];
  if(data != nullptr){
    memcpy(this->data, data, w*h*channels*component_size);
  }
}

Image::Image(int w, int h, int channels, size_t component_size) : width(w), height(h), channels(channels), component_size(component_size){
  this->data = new uint8_t[w*h*channels*component_size];
}

ExternalImage::ExternalImage(string path, bool delay_load, int expected_channels) : path(path) {
	if (!delay_load) {
		this->pload(expected_channels);
	}
}
ExternalImage::ExternalImage(const char* const path, bool delay_load, int expected_channels) : path(path) {
	if (!delay_load) {
		this->pload(expected_channels);
	}
}

ExternalImageHDR::ExternalImageHDR(string path, bool delay_load, int expected_channels) : ExternalImage(path, true) {
	if (!delay_load) {
		pload(expected_channels);
	}
}
ExternalImageHDR::ExternalImageHDR(const char* const path, bool delay_load, int expected_channels) : ExternalImage(path, true){
	if (!delay_load) {
		pload(expected_channels);
	}
}

bool ExternalImage::pload(int expected_channels){
  int x,y,n;
  x = y = n = 0; 
  int request_channels = expected_channels;
  if(expected_channels <= 0){
    request_channels = 4;
  }
  stbi_set_flip_vertically_on_load(true);
  data = (uint8_t*) stbi_load(path.c_str(), &x, &y, &n, request_channels);
  if(!data){
    stb_fail_reason = stbi_failure_reason();
    loaded = false;
    return(loaded);
  }
  if(n != request_channels && expected_channels > 0){
    fprintf(stderr, "Warning! Image provided reportedly has %i channels. Only %i channels were requested so results may not be as expected\n", n, expected_channels);
  }
  if(x < 0 || y < 0){
    fprintf(stderr, "STBI failed critically\n");
    exit(-1);
  }
  width = static_cast<int>(x);
  height = static_cast<int>(y);
  component_size = sizeof(uint8_t);
  channels = n;

  loaded = true;
  return(true);
}

bool ExternalImageHDR::pload(int expected_channels){
  int x,y,n;
  x = y = n = 0; 
  int request_channels = expected_channels;
  if(expected_channels <= 0){
    request_channels = 4;
  }
  stbi_set_flip_vertically_on_load(true);
  data = (uint8_t*) stbi_loadf(path.c_str(), &x, &y, &n, request_channels);
  if(!data){
    stb_fail_reason = stbi_failure_reason();
    loaded = false;
    return(loaded);
  }
  if(n != request_channels && expected_channels > 0){
    fprintf(stderr, "Warning! Image provided reportedly has %i channels. Only %i channels were requested so results may not be as expected\n", n, expected_channels);
  }
  if(x < 0 || y < 0){
    fprintf(stderr, "STBI failed critically\n");
    exit(-1);
  }
  width = static_cast<int>(x);
  height = static_cast<int>(y);
  channels = n;
  component_size = sizeof(float);

  loaded = true;
  return(true);
}

int Image::writeFile(const string& path) const{
  stbi_flip_vertically_on_write(true);
  if(!data){
		fprintf(stderr, "Error: Tried to output image with no data!\n");
    return(1);
	}else if(!stbi_write_png(path.c_str(), width, height, channels, data, 0)){
		fprintf(stderr, "Outputting image failed for unspecified reason!\n");
    return(2);
	}
	return(0);
}

int Image::writeFileFormat(const string& path, ImageWriteFormat format) const{
  stbi_flip_vertically_on_write(true);
  if(!data){
		fprintf(stderr, "Error: Tried to output image with no data!\n");
    return(1);
	}
  int result = 0;
  switch(format){
    case Image::ImageWriteFormat::PNG:
      result = stbi_write_png(path.c_str(), width, height, channels, (uint8_t*) data, 0);
      break;
    case Image::ImageWriteFormat::HDR:
      result = stbi_write_hdr(path.c_str(), width, height, channels, (float*) data);
      break;
    case Image::ImageWriteFormat::JPEG:
      result = stbi_write_jpg(path.c_str(), width, height, channels, (uint8_t*) data, 97);
      break;
    default:
      result = -1;
  }
  if(!result){
    fprintf(stderr, "Outputting image failed for unspecified reason!\n");
    return(2);
  }
	return(0);
}

void ExternalImage::_free_image_data(){
	stbi_image_free(data);
}
