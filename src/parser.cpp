#include <bitset>
#include "gif.hpp"

gif::Header::Header(std::ifstream &stream) {
  stream.read(this->signature, 3);
  stream.read(this->version, 3);
  const auto sig = this->signature;
  if (sig[0] != 'G' || sig[1] != 'I' || sig[2] != 'F') {
    throw std::runtime_error("Invalid gif supplied");
  }
}


gif::Descriptor::Descriptor(std::ifstream &stream) {
  stream.read(reinterpret_cast<char *>(&width), 2);
  stream.read(reinterpret_cast<char *>(&height), 2);
  unsigned char packed = stream.get();
  globalColorTable = packed & 0b10000000u;
  colorResolution = packed & 0b01110000u;
  sortFlag = packed & 0b00001000u;
  colorTableSize = packed & 0b00000111u;
  stream.read(reinterpret_cast<char *>(&bgColorIndex), 2);
  stream.read(reinterpret_cast<char *>(&pixelAspectRatio), 2);
}

gif::GlobalColorTable::GlobalColorTable(unsigned char count, std::ifstream &stream) {
  constexpr int READ_PER_COLOR = 3;
  if (count < 0) {
    throw std::runtime_error("Invalid count for color table: " + std::to_string(count));
  }
  const int consumeAmount = READ_PER_COLOR << (count + 1);
  const int colorCount = consumeAmount / READ_PER_COLOR;
  colors.reserve(colorCount);
  for (int i = 0; i < colorCount; i++) {
    unsigned char r = stream.get();
    unsigned char g = stream.get();
    unsigned char b = stream.get();
    const auto currentColor = Color{.red = r, .green = g, .blue = b};
    colors.push_back(currentColor);
  }
  auto q = 1;
}