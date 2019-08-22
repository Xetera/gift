#include <bitset>
#include <parser.hpp>
#include "gif.hpp"

using std::ifstream;

void withSubBlocks(ifstream &stream,
                   const std::function<void(UCharsVec &, unsigned char)> &f) {
  unsigned char blockCount;
  while(((blockCount = stream.get()) != gif::BLOCK_TERMINATOR)) {
    UCharsVec byteChunks;
    byteChunks.reserve(blockCount);
    for(int i = 0; i < blockCount; i++) {
      byteChunks.emplace_back(stream.get());
    }
    f(byteChunks, blockCount);
  }
}
void consumeExtIntroducer(ifstream &stream) {
  const unsigned char extIntroducer = stream.get();
  if(extIntroducer != gif::EXTENSION_INTRODUCER) {
    throw std::runtime_error(
        "Extension block does not start with valid identifier");
  }
}

bool hasColorTable(ifstream &stream) {
  constexpr char BIT_LENGTH                = 2;
  constexpr UChars<BIT_LENGTH> TARGET_BITS = { gif::EXTENSION_INTRODUCER,
                                               gif::GraphicsControl::LABEL };
  UChars<BIT_LENGTH> bits{};
  const auto position = stream.tellg();
  stream.read((char *) bits.data(), BIT_LENGTH);
  stream.seekg(position);
  return bits != TARGET_BITS;
}

gif::Header::Header(ifstream &stream) {
  static constexpr int SIGNATURE_LENGTH                    = 3;
  static constexpr int VERSION_LENGTH                      = 3;
  static constexpr const Chars<VERSION_LENGTH> _89a        = { '8', '9', 'a' };
  static constexpr const Chars<VERSION_LENGTH> _87a        = { '8', '7', 'a' };
  static constexpr Chars<SIGNATURE_LENGTH> VALID_SIGNATURE = { 'G', 'I', 'F' };

  Chars<SIGNATURE_LENGTH> signature{};
  stream.read(signature.data(), SIGNATURE_LENGTH);
  const auto isValidGif = VALID_SIGNATURE == signature;

  if(!isValidGif) {
    throw std::runtime_error("File is not a gif");
  }

  Chars<VERSION_LENGTH> versionBuf{};
  stream.read(versionBuf.data(), VERSION_LENGTH);

  if(versionBuf == _87a) {
    version = gif::Version::_87a;
  } else if(versionBuf == _89a) {
    version = gif::Version::_89a;
  } else {
    const auto versionStr =
        std::string(std::begin(versionBuf), std::end(versionBuf));
    throw std::runtime_error("File has invalid gif version: " + versionStr);
  }
}

gif::Descriptor::Descriptor(ifstream &stream) {
  stream.read((char *) (&width), 2);
  stream.read((char *) (&height), 2);
  unsigned char packed = stream.get();
  globalColorTable     = packed & 0b1u;
  colorResolution      = packed & 0b0111u;
  sortFlag             = packed & 0b00001u;
  colorTableSize       = packed & 0b00000111u;
  bgColorIndex         = stream.get();
  pixelAspectRatio     = stream.get();
}

gif::ColorTable::ColorTable(unsigned char count, ifstream &stream) {
  constexpr auto BYTE_PER_COLOR = 3;
  if(count < 0) {
    throw std::runtime_error("Invalid count for color table: " +
                             std::to_string(count));
  }
  const int consumeAmount = BYTE_PER_COLOR << (count + 1u);
  const int colorCount    = consumeAmount / BYTE_PER_COLOR;
  colors.reserve(colorCount);
  for(int i = 0; i < colorCount; i++) {
    const unsigned char r = stream.get();
    const unsigned char g = stream.get();
    const unsigned char b = stream.get();
    colors.emplace_back(Color{ r, g, b });
  }
}

gif::GraphicsControl::GraphicsControl(ifstream &stream) {
  consumeExtIntroducer(stream);
  consumeLabel<gif::GraphicsControl::LABEL>(stream);
  byteSize                   = stream.get();
  const unsigned char packed = stream.get();
  // Discarding the first 3 bits, reserved for future use
  disposalMethod   = packed & 0b00011100u;
  userInput        = packed & 0b00000010u;
  transparentColor = packed & 0b00000001u;
  stream.read((char *) (&delayTime), 2);
  transparentColorIndex = stream.get();
  consumeLabel<gif::BLOCK_TERMINATOR>(stream);
}

gif::ImageDescriptor::ImageDescriptor(ifstream &stream) {
  consumeLabel<gif::ImageDescriptor::SEPARATOR>(stream);
  stream.read((char *) (&left), 2);
  stream.read((char *) (&top), 2);
  stream.read((char *) (&width), 2);
  stream.read((char *) (&height), 2);
  unsigned char packed = stream.get();
  haslocalColorTable   = packed & 0b10000000u;
  isInterlacing        = packed & 0b01000000u;
  isSorted             = packed & 0b00100000u;
  localColorTableSize  = packed & 0b00000111u;
}

gif::ImageSubData::ImageSubData(unsigned char count, UCharsVec &bytes)
    : byteCount(count), imageBytes(bytes) {}

gif::CompressedImageData::CompressedImageData(ifstream &stream) {
  constexpr unsigned char AVERAGE_SUB_BLOCK_SIZE = 8;
  minimumCodeSize                                = stream.get();
  // The size of the following sub-blocks aren't known before parsing.
  // Chances are the sub-blocks aren't just going to be empty
  // Just guessing the sizes here, the average block is likely larger
  // than just 8 bytes but it doesn't really matter
  subBlocks.reserve(minimumCodeSize * AVERAGE_SUB_BLOCK_SIZE);
  withSubBlocks(stream, [this](auto &byteChunks, auto blockCount) {
    const auto subData = ImageSubData(blockCount, byteChunks);
    subBlocks.emplace_back(subData);
  });
}