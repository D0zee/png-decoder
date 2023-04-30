#pragma once

#include <vector>

struct IHDR {
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t c_method;
    uint8_t f_method;
    uint8_t i_method;
};

struct Chunk {
    uint32_t length;
    char type[4];
    std::vector<uint8_t> data;
    uint32_t crc;
};

