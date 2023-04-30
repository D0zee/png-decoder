#pragma once

#include <boost/crc.hpp>
#include <vector>

struct CrcData {
    const char *data;
    std::size_t len;
};

uint32_t calculateCrcFromChunk(const std::vector<CrcData> &chunkInfo);