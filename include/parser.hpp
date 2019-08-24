#pragma once

#include <array>
#include <fstream>
#include <functional>
#include "gif.hpp"

gif::CompressedImage parseGif(std::ifstream &);

std::vector<gif::ImageBody> parseBody(gif::ImageMetadata &, std::ifstream &);

void peekN(char *data, int length, std::ifstream &stream);

template <class T, uint8_t LABEL>
std::optional<T> tryParseOptionalExtension(std::ifstream &stream) {
  uint8_t data = stream.peek();
  const auto isGraphicsControl = data == LABEL;
  // I don't understand how this constructor isn't throwing an error here lol
  return isGraphicsControl ? std::make_optional(T(stream)) : std::nullopt;
}
/**
 * Checks whether or not the remaining stream
 * includes an optional color table or not
 * @param stream
 * @return bool
 */
bool hasColorTable(std::ifstream &stream);

void withSubBlocks(std::ifstream &, const std::function<void(UCharsVec &)> &);

/**
 * Consumes a block label that's a compile time constant
 * @throws std::runtime_error
 * @tparam LABEL
 */
template <int LABEL>
void consumeLabel(std::ifstream &stream) {
  const unsigned char extractedLabel = stream.get();
  if(extractedLabel != LABEL) {
    throw std::runtime_error(
        "Consumed label " + std::to_string(extractedLabel) +
        " doesn't match expected constant " + std::to_string(LABEL));
  }
}

/**
 * Parses the boilerplate of the extension block
 * @tparam T
 * @param stream
 * @param f
 */
template <int T>
void withExtension(std::ifstream &stream,
                   const std::function<void(std::ifstream &)> &f) {
  consumeLabel<gif::EXTENSION_INTRODUCER>(stream);
  consumeLabel<T>(stream);
  f(stream);
  const unsigned char nextByte = stream.peek();
  if(nextByte == gif::BLOCK_TERMINATOR) {
    consumeLabel<gif::BLOCK_TERMINATOR>(stream);
  }
}
