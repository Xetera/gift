#pragma once

#include <fstream>
#include <functional>
#include <vector>
#include <memory>
#include <list>

namespace gif {

/**
 * Header received at the very beginning
 * of the file that describes the file structure
 */
struct Header {
  /**
   * Always GIF
   */
  char signature[3];
  /**
   * This one is probably not going to change
   */
  char version[3];
  explicit Header(std::ifstream &);
};

struct Descriptor {
  unsigned short width;
  unsigned short height;
  unsigned char bgColorIndex;
  unsigned char pixelAspectRatio;
  /**
   * Extracted from the packed bit
   */
  unsigned char globalColorTable;
  unsigned char colorResolution;
  unsigned char sortFlag;
  unsigned char colorTableSize;
  explicit Descriptor(std::ifstream &);
};

struct GlobalColorTable {
  std::vector<unsigned char[3]> color;
  explicit GlobalColorTable(unsigned char, std::ifstream&);
};

struct Image {
  Header header;
  Descriptor descriptor;
};


}  // namespace gif

std::ifstream readGifRaw(const std::string &);
