#include "CrcChecker.h"

constexpr auto BITS = 32; // width from docs
constexpr auto TRUNC_POLY = 0x4C11DB7;
constexpr auto INIT_REM = 0xFFFFFFFF; // CRC initialized to all 1's
constexpr auto FINAL_XOR = 0xFFFFFFFF;
constexpr auto REFLECT_IN = true;
constexpr auto REFLECT_REM = true;

uint32_t calculateCrcFromChunk(const std::vector<CrcData> &chunkInfo) {
    boost::crc_optimal<BITS, TRUNC_POLY, INIT_REM, FINAL_XOR, REFLECT_IN, REFLECT_REM> crc;
    for (auto bytes: chunkInfo) {
        crc.process_bytes(bytes.data, bytes.len);
    }
    return crc.checksum();
}