#include "parser.hpp"
#include <bitset>
#include "gif.hpp"
#include "typedefs.hpp"

using std::ifstream;

void peekN(char *data, int length, std::ifstream &stream) {
  const auto position = stream.tellg();
  stream.read(data, length);
  stream.seekg(position);
}

std::vector<gif::ImageBody> parseBody(gif::ImageMetadata &meta,
                                      std::ifstream &stream) {
  const uint8_t firstByte  = stream.get();
  const auto isInvalidByte = firstByte != gif::EXTENSION_INTRODUCER &&
                             firstByte != gif::ImageDescriptor::SEPARATOR;
  if(firstByte == gif::TRAILER) {
    return std::vector<gif::ImageBody>();
  } else if(isInvalidByte) {
    throw std::runtime_error("Got unexpected value " +
                             std::to_string(firstByte));
  }
  const uint8_t label = stream.peek();
  if(label == gif::ApplicationExtension::LABEL) {
    gif::ImageBody app_ = gif::ApplicationExtension(stream);
    auto out            = parseBody(meta, stream);
    out.push_back(app_);
    return out;
  } else if(label == gif::CommentExtension::LABEL) {
    gif::ImageBody comment = gif::CommentExtension(stream);
    auto out               = parseBody(meta, stream);
    out.push_back(comment);
    return out;
  }
  gif::ImageBody block = gif::ImageBlock(stream);
  auto out             = parseBody(meta, stream);
  out.push_back(block);
  return out;
}

gif::CompressedImage parseGif(std::ifstream &stream) {
  gif::Header header(stream);
  gif::ScreenDescriptor descriptor(stream);
  const auto hasTable = hasColorTable(stream);
  gif::OptionalColorTable globalColorTable =
      hasTable ? std::make_optional(
                     gif::ColorTable(descriptor.colorTableSize, stream))
               : std::nullopt;

  gif::ImageMetadata meta(header, descriptor, globalColorTable);

  const auto body = parseBody(meta, stream);
  return gif::CompressedImage(meta, body);
};

void withSubBlocks(ifstream &stream,
                   const std::function<void(UCharsVec &)> &f) {
  unsigned char blockCount;
  while(((blockCount = stream.get()) != gif::BLOCK_TERMINATOR)) {
    UCharsVec byteChunks;
    byteChunks.reserve(blockCount);
    for(int i = 0; i < blockCount; i++) {
      byteChunks.emplace_back(stream.get());
    }
    f(byteChunks);
  }
}

bool hasColorTable(ifstream &stream) {
  constexpr char BIT_LENGTH                = 2;
  constexpr UChars<BIT_LENGTH> TARGET_BITS = { gif::EXTENSION_INTRODUCER,
                                               gif::GraphicsControl::LABEL };
  UChars<BIT_LENGTH> bits{};
  peekN((char *) bits.data(), BIT_LENGTH, stream);
  return bits != TARGET_BITS;
}

gif::Header::Header(ifstream &stream) {
  static constexpr int SIGNATURE_LENGTH                     = 3;
  static constexpr int VERSION_LENGTH                       = 3;
  static constexpr const UChars<VERSION_LENGTH> _89a        = { '8', '9', 'a' };
  static constexpr const UChars<VERSION_LENGTH> _87a        = { '8', '7', 'a' };
  static constexpr UChars<SIGNATURE_LENGTH> VALID_SIGNATURE = { 'G', 'I', 'F' };

  UChars<SIGNATURE_LENGTH> signature{};
  stream.read((char *) signature.data(), SIGNATURE_LENGTH);
  const auto isValidGif = VALID_SIGNATURE == signature;

  if(!isValidGif) {
    throw std::runtime_error("File is not a gif");
  }

  UChars<VERSION_LENGTH> versionBuf{};
  stream.read((char *) versionBuf.data(), VERSION_LENGTH);

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

gif::FrameData getFrameData(ifstream &stream) {
  const uint8_t initial = stream.peek();
  const auto isImage    = initial == gif::ImageDescriptor::SEPARATOR;
  if(isImage) {
    gif::ImageDescriptor descriptor(stream);
    std::optional<gif::ColorTable> localTable =
        descriptor.haslocalColorTable
            ? std::make_optional(
                  gif::ColorTable(descriptor.localColorTableSize, stream))
            : std::nullopt;
    gif::CompressedImageData data(stream);
    return gif::Frame(descriptor, localTable, data);
  }
  return gif::PlainTextExtension(stream);
}

gif::PlainTextExtension::PlainTextExtension(ifstream &stream) {
  stream.ignore(BLOCK_SIZE + 2);
  stream.ignore(BLOCK_SIZE, gif::BLOCK_TERMINATOR);
}

gif::ImageBlock::ImageBlock(std::ifstream &stream)
    : graphicsControl(
          tryParseOptionalExtension<gif::GraphicsControl,
                                    gif::GraphicsControl::LABEL>(stream)),
      frameData(getFrameData(stream)) {}

gif::ScreenDescriptor::ScreenDescriptor(ifstream &stream) {
  stream.read((char *) (&width), 2);
  stream.read((char *) (&height), 2);
  uint8_t packed      = stream.get();
  hasGlobalColorTable = packed & 0b1u;
  colorResolution     = packed & 0b0111u;
  isSorted            = packed & 0b00001u;
  colorTableSize      = packed & 0b00000111u;
  bgColorIndex        = stream.get();
  pixelAspectRatio    = stream.get();
}

gif::ColorTable::ColorTable(unsigned char count, ifstream &stream) {
  constexpr auto BYTE_PER_COLOR = 3;
  if(count < 0) {
    const auto countStr = std::to_string(count);
    throw std::runtime_error("Invalid count for color table: " + countStr);
  }
  const int consumeAmount = BYTE_PER_COLOR << (count + 1u);
  const int colorCount    = consumeAmount / BYTE_PER_COLOR;
  colors.reserve(colorCount);
  for(int i = 0; i < colorCount; i++) {
    const uint8_t r = stream.get();
    const uint8_t g = stream.get();
    const uint8_t b = stream.get();
    colors.emplace_back(r, g, b);
  }
}

gif::GraphicsControl::GraphicsControl(ifstream &stream) {
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

gif::ImageSubData::ImageSubData(UCharsVec &bytes) : imageBytes(bytes) {}

gif::CompressedImageData::CompressedImageData(ifstream &stream) {
  constexpr unsigned char AVERAGE_SUB_BLOCK_SIZE = 8;
  minimumCodeSize                                = stream.get();
  // The size of the following sub-blocks aren't known before parsing.
  // Chances are the sub-blocks aren't just going to be empty
  // Just guessing the sizes here, the average block is likely larger
  // than just 8 bytes but it doesn't really matter
  subBlocks.reserve(minimumCodeSize * AVERAGE_SUB_BLOCK_SIZE);
  withSubBlocks(
      stream, [this](auto &byteChunks) { subBlocks.emplace_back(byteChunks); });
}

gif::ApplicationExtension::ApplicationExtension(std::ifstream &stream) {
  consumeLabel<ApplicationExtension::LABEL>(stream);
  // I think this data length is not variable but then... why does it even
  // take up a byte in the first place? it's all very weird. GIF is weird...
  consumeLabel<ApplicationExtension::APPLICATION_DATA_LENGTH>(stream);
  stream.read((char *) applicationData.data(), APPLICATION_DATA_LENGTH);
  // I have no idea if these are constant
  consumeLabel<0x03>(stream);
  consumeLabel<0x01>(stream);
  stream.read((char *) (&loopCount), 2);
  consumeLabel<gif::BLOCK_TERMINATOR>(stream);
}

gif::CommentExtension::CommentExtension(std::ifstream &stream) {
  withSubBlocks(stream, [this](const auto &letters) {
    comments.emplace_back(letters.begin(), letters.end());
  });
}