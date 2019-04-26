#pragma once
#ifndef LVPM_IMAGE_H_
#define LVPM_IMAGE_H_

#include <string>
#include <stdint.h>
#include <common.h>

using namespace std;

class Image{
public:
  Image(void* const data, int w, int h, int channels, size_t component_size = sizeof(uint8_t));
  Image(int w, int h, int channels, size_t component_size = sizeof(uint8_t));
  ~Image() {_free_image_data();}
  virtual void _free_image_data(){delete[] data;}
  int getWidth() const {return(width);}
  int getHeight() const {return(height);}
  int getChannels() const {return(channels);}
  size_t getComponentSize() const {return(component_size);}
  size_t getTotalDataSize() const {return(component_size*channels*width*height);}

  enum class ImageWriteFormat {PNG,HDR,JPEG};

  int writeFile(const string& path) const;
  int writeFile(const char* path) const {return(writeFile(string(path)));}
  int writeFileFormat(const string& path, ImageWriteFormat format) const;
  int writeFileFormat(const char* path, ImageWriteFormat format) const {return(writeFileFormat(string(path), format));}

  uint8_t* data = nullptr;
protected:
	Image() {}

  int channels;
  int width, height;
  size_t component_size;
};

class ExternalImage : public Image{
public:
	ExternalImage(string path, bool delay_load = false, int expected_channels=-1);
	ExternalImage(const char* const path, bool delay_load = false, int expected_channels=-1);
  virtual void _free_image_data();

  bool load(int expected_channels=-1) {
    if(loaded){fprintf(stderr, "Warning! Tried to load already loaded image.\n"); return(false);}
    else{return(pload(expected_channels));}
  };

  const string& getPath() const {return(path);}
  const bool isLoaded() const {return(loaded);}

  const char* const getErrorMsg() const {if(stb_fail_reason){return(stb_fail_reason);}}
  virtual void printErrorMsg() const {if(stb_fail_reason){fprintf(stderr, "Image load failed: '%s'\n", stb_fail_reason);}}
protected:
  virtual bool pload(int expected_channels);
  bool loaded = false;
  string path;
  const char* stb_fail_reason = nullptr;
};

class ExternalImageHDR : public ExternalImage{
public:
  ExternalImageHDR(string path, bool delay_load = false, int expected_channels=-1);
	ExternalImageHDR(const char* const path, bool delay_load = false, int expected_channels=-1);
  void printErrorMsg() const override {if(stb_fail_reason){fprintf(stderr, "HDR Image load failed: '%s'\n", stb_fail_reason);}}
protected:
  bool pload(int expected_channels) override;
};

#endif