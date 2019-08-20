#pragma once

#include <fstream>
#include <functional>
#include <list>
#include <memory>
#include <vector>
#include <optional>

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
  explicit Header(std::ifstream&);
};

struct Descriptor {
  /**
   * width, up to 65,535px
   */
  unsigned short width;
  /**
   * height, up to 65,535px
   */
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
  explicit Descriptor(std::ifstream&);
};

struct Color {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
};

struct GlobalColorTable {
  std::vector<Color> colors;
  explicit GlobalColorTable(unsigned char, std::ifstream&);
};

/**
 * All graphics control extensions
 * begin with   | 21 F9
 * and end with | 00
 */
struct GraphicsControl {
  unsigned char byteSize;
  unsigned char disposalMethod;
  unsigned char userInput;
  unsigned char transparentColor;
  unsigned char delayTime;
  unsigned char transparentColorIndex;
};

struct ImageDescriptor {
  
};

struct Image {
  Header header;
  Descriptor descriptor;
  std::optional<gif::GlobalColorTable> globalColorTable;
  GraphicsControl graphicsControl;
};

}  // namespace gif

std::ifstream readGifRaw(const std::string&);
