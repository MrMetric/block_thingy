#pragma once

#include <stdint.h> // kinda lame including this here, but whatever

// signed instead of std::size_t to allow comparison with signed values
constexpr int_fast32_t CHUNK_SIZE = 32;
constexpr int_fast32_t CHUNK_BLOCK_COUNT = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

class Chunk;
