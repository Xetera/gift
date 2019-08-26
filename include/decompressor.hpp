#pragma once

#include "gif.hpp"

std::vector<gif::ImageBlock> extractFrames(const std::vector<gif::ImageBody>&);

std::vector<uint8_t> compress(const std::string&);
void decompress(const gif::CompressedImageData&);
