#include "gif.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include <vector>
#include <decompressor.hpp>
#include "parser.hpp"

std::ifstream readGifRaw(const std::string &file) {
  std::ifstream fs(file, std::ios::binary);
  if (!fs.is_open()) {
    throw std::runtime_error("file cannot be found: " + file);
  }
  return fs;
}

int main() {
  auto stream = readGifRaw("./sample_1.gif");
  const auto gif = parseGif(stream);
  const std::string s = "banana_bandana";
  std::vector<uint8_t> e(s.begin(), s.end());
  compress(s);
  auto q = 1+1;
//  const auto header = gif::Header(stream);
//
//  const auto descriptor = gif::ScreenDescriptor(stream);
//  const auto hasTable = hasColorTable(stream);
//  const std::optional<gif::ColorTable> globalColorTable = hasTable
//    ? std::make_optional( gif::ColorTable(descriptor.colorTableSize, stream))
//    : std::nullopt;
//  const auto graphicsControl = gif::GraphicsControl(stream);
//  const auto imageDescriptor = gif::ImageDescriptor(stream);
//  const std::optional<gif::ColorTable> localColorTable = imageDescriptor.haslocalColorTable
//    ? std::make_optional(gif::ColorTable(imageDescriptor.localColorTableSize, stream))
//    : std::nullopt;
//  const auto imageData = gif::ImageData(stream);
//  consumeLabel<gif::TRAILER>(stream);
}

