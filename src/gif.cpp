#include "gif.hpp"
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <parser.hpp>

void writeGif(const std::string &name, const gif::Image &gif) {
  const auto bytes = gif.header.version;  // + gif.descriptor;
  std::ofstream fstream(name, std::ios_base::binary);
  fstream << bytes;
}

std::ifstream readGifRaw(const std::string &file) {
  std::ifstream fs(file, std::ios::binary);
  if (!fs.is_open()) {
    throw std::runtime_error("file cannot be found: " + file);
  }
  return fs;
}

int main() {
  auto stream = readGifRaw("./newgame.gif");
  const auto header = gif::Header(stream);
  const auto descriptor = gif::Descriptor(stream);
  const auto table = gif::GlobalColorTable(descriptor.colorTableSize, stream);
  std::cout << header.signature;
}