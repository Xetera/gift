#include "gif.hpp"
#include <functional>
#include <iostream>
#include <memory>
#include "parser.hpp"

//void writeGif(const std::string &name, const gif::ImageMetadata &gif) {
//  const auto bytes = gif.header.version;  // + gif.descriptor;
//  std::ofstream fstream(name, std::ios_base::binary);
//  fstream << bytes;
//}

std::ifstream readGifRaw(const std::string &file) {
  std::ifstream fs(file, std::ios::binary);
  if (!fs.is_open()) {
    throw std::runtime_error("file cannot be found: " + file);
  }
  return fs;
}

int main() {
  auto stream = readGifRaw("./chino.gif");
  const auto header = gif::Header(stream);
  const auto descriptor = gif::Descriptor(stream);
  const auto hasTable = hasColorTable(stream);
  const std::optional<gif::ColorTable> globalColorTable = hasTable
    ? std::make_optional( gif::ColorTable(descriptor.colorTableSize, stream))
    : std::nullopt;
  const auto graphicsControl = gif::GraphicsControl(stream);
  const auto imageDescriptor = gif::ImageDescriptor(stream);
  const std::optional<gif::ColorTable> localColorTable = imageDescriptor.haslocalColorTable
    ? std::make_optional(gif::ColorTable(imageDescriptor.localColorTableSize, stream))
    : std::nullopt;
  const auto imageData = gif::CompressedImageData(stream);
  consumeLabel<gif::TRAILER>(stream);
}
