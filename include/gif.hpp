#pragma once

#include <filesystem>
#include <fstream>
#include <functional>
#include <list>
#include <optional>
#include <utility>
#include <variant>
#include <vector>
#include "typedefs.hpp"

namespace gif {
constexpr uint8_t EXTENSION_INTRODUCER = 0x21;
constexpr uint8_t BLOCK_TERMINATOR     = 0x00;
constexpr uint8_t TRAILER              = 0x3B;

struct Extension {};

enum class Version { _89a, _87a };

/**
 * Header received at the very beginning
 * of the file that describes the file structure
 */
struct Header {
  Version version;
  explicit Header(std::ifstream &);
};

struct ScreenDescriptor {
  /**
   * width, up to 65,535px
   */
  uint16_t width;
  /**
   * height, up to 65,535px
   */
  uint16_t height;
  uint8_t bgColorIndex;
  uint8_t pixelAspectRatio;
  uint8_t colorResolution;
  uint8_t colorTableSize;
  /**
   * Extracted from the packed bit
   */
  bool hasGlobalColorTable;
  bool isSorted;
  explicit ScreenDescriptor(std::ifstream &);
};

/**
 * 3 byte color block extracted from [gif::ColorTable]
 */
struct Color {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  explicit Color(uint8_t red, uint8_t blue, uint8_t green)
      : red(red), green(green), blue(blue) {}
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
struct GraphicsControl : Extension {
  static constexpr uint8_t LABEL = 0xF9;
  uint8_t byteSize;
  uint8_t disposalMethod;
  uint8_t userInput;
  uint8_t transparentColor;
  uint8_t delayTime;
  uint8_t transparentColorIndex;
  explicit GraphicsControl(std::ifstream &);
};

/**
 * A descriptor for a single image
 */
struct ImageDescriptor {
  static constexpr uint8_t SEPARATOR = 0x2C;
  uint16_t left;
  uint16_t top;
  uint16_t width;
  uint16_t height;
  bool haslocalColorTable;
  bool isInterlacing;
  bool isSorted;
  uint8_t localColorTableSize;
  explicit ImageDescriptor(std::ifstream &);
};

/**
 * Chunks of image data found in the [gif::ImageData] block
 */
struct ImageSubData {
  std::vector<uint8_t> imageBytes;
  explicit ImageSubData(std::vector<unsigned char> &);
};

/**
 * LZW compressed image data of the gif
 */
struct CompressedImageData {
  uint8_t minimumCodeSize;
  std::vector<gif::ImageSubData> subBlocks;
  explicit CompressedImageData(std::ifstream &);
};

/**
 * Decompressed GIF image data
 */
struct DecompressedImageData {
  uint8_t minimumCodeSize;
  std::vector<gif::ImageSubData> subBlocks;
  //  explicit CompressedImageData(std::ifstream&);
};

// nobody cares about this, just skipping it for now
struct PlainTextExtension {
  static constexpr uint8_t LABEL      = 0x01;
  static constexpr uint8_t BLOCK_SIZE = 0x0C;
  explicit PlainTextExtension(std::ifstream &);
};

struct ApplicationExtension {
  static constexpr uint8_t LABEL                    = 0xFF;
  static constexpr uint16_t APPLICATION_DATA_LENGTH = 0x0B;
  // Typically just "NETSCAPE2.0"
  UChars<APPLICATION_DATA_LENGTH> applicationData;
  uint16_t loopCount;
  explicit ApplicationExtension(std::ifstream &);
};

struct CommentExtension {
  static constexpr uint8_t LABEL = 0xFE;
  std::vector<std::string> comments;
  explicit CommentExtension(std::ifstream &);
};

using OptionalColorTable = std::optional<ColorTable>;

struct Frame {
  ImageDescriptor imageDescriptor;
  OptionalColorTable colorTable;
  CompressedImageData imageData;
  explicit Frame(const ImageDescriptor &descriptor,
                 const OptionalColorTable &colorTable,
                 const CompressedImageData &data)
      : imageDescriptor(descriptor), colorTable(colorTable), imageData(data){};
};

using FrameData = std::variant<Frame, PlainTextExtension>;

struct ImageBlock {
  std::optional<GraphicsControl> graphicsControl;
  FrameData frameData;
  explicit ImageBlock(std::ifstream &);
};

using ImageBody =
    std::variant<ImageBlock, ApplicationExtension, CommentExtension>;
/**
 * The metadata of a GIF
 */
struct ImageMetadata {
  Header header;
  ScreenDescriptor descriptor;
  OptionalColorTable globalColorTable;
  explicit ImageMetadata(const Header &header,
                         const ScreenDescriptor &descriptor,
                         const OptionalColorTable &table)
      : header(header), descriptor(descriptor), globalColorTable(table) {}
};

struct CompressedImage {
  ImageMetadata metadata;
  std::vector<ImageBody> compressedImageData;
  explicit CompressedImage(const ImageMetadata &meta,
                           const std::vector<ImageBody> &body)
      : metadata(meta), compressedImageData(body) {}
};

struct DecompressedImage {
  ImageMetadata metadata;
  DecompressedImageData decompressedImageData;
  explicit DecompressedImage(const ImageMetadata &);
};
}  // namespace gif

std::ifstream readGifRaw(const std::string &);
