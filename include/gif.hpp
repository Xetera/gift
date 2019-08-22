#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <list>
#include <memory>
#include <optional>
#include <vector>
#include "parser.hpp"

namespace gif {
constexpr unsigned char EXTENSION_INTRODUCER = 0x21;
constexpr unsigned char BLOCK_TERMINATOR     = 0x00;
constexpr unsigned char TRAILER              = 0x3B;

enum class Version { _89a, _87a };

/**
 * Header received at the very beginning
 * of the file that describes the file structure
 */
struct Header {
  Version version;

  explicit Header(std::ifstream &);
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

  explicit Descriptor(std::ifstream &);
};

/**
 * 3 byte color block extracted from [gif::ColorTable]
 */
struct Color {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
};

/**
 * Color tables, found in both under the descriptor as
 * a global color table and after a frame
 */
struct ColorTable {
  std::vector<Color> colors;

  explicit ColorTable(unsigned char, std::ifstream &);
};

/**
 * All graphics control extensions
 * The constructor is only responsible for parsing
 * past the labels. It expects for the stream cursor
 * to have parsed the following labels.
 * | 21 F9 |.
 */
struct GraphicsControl {
  static constexpr unsigned char LABEL = 0xF9;
  unsigned char byteSize;
  unsigned char disposalMethod;
  unsigned char userInput;
  unsigned char transparentColor;
  unsigned char delayTime;
  unsigned char transparentColorIndex;

  explicit GraphicsControl(std::ifstream &);
};

/**
 * A descriptor for a single image
 */
struct ImageDescriptor {
  static constexpr char SEPARATOR = 0x2C;
  unsigned short left;
  unsigned short top;
  unsigned short width;
  unsigned short height;
  unsigned char haslocalColorTable;
  unsigned char isInterlacing;
  unsigned char isSorted;
  unsigned char localColorTableSize;

  explicit ImageDescriptor(std::ifstream &);
};

/**
 * Chunks of image data found in the [gif::ImageData] block
 */
struct ImageSubData {
  unsigned char byteCount;
  std::vector<unsigned char> imageBytes;

  explicit ImageSubData(unsigned char, std::vector<unsigned char> &);
};

/**
 * LZW compressed image data of the gif
 */
struct CompressedImageData {
  unsigned char minimumCodeSize;
  std::vector<gif::ImageSubData> subBlocks;

  explicit CompressedImageData(std::ifstream &);
};

/**
 * Decompressed GIF image data
 */
struct DecompressedImageData {
  unsigned char minimumCodeSize;
  std::vector<gif::ImageSubData> subBlocks;

  //  explicit CompressedImageData(std::ifstream&);
};

struct ApplicationExtension {

};

/**
 * The metadata of a GIF
 */
struct ImageMetadata {
  Header header;
  Descriptor descriptor;
  std::optional<gif::ColorTable> globalColorTable;
  GraphicsControl graphicsControl;
  ImageDescriptor imageDescriptor;
  std::optional<gif::ColorTable> localColorTable;

  /**
   * Reading images from the filesystem
   * @param path
   */
  explicit ImageMetadata(const std::filesystem::path &);
};

struct CompressedImage {
  ImageMetadata metadata;
  CompressedImageData compressedImageData;

  explicit CompressedImage(const ImageMetadata &);
};

struct DecompressedImage {
  ImageMetadata metadata;
  DecompressedImageData decompressedImageData;

  explicit DecompressedImage(const ImageMetadata &);
};
}  // namespace gif

std::ifstream readGifRaw(const std::string &);
