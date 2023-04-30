#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include <iostream>
#include <cstring>
#include "chunks.h"
#include <unordered_map>
#include <set>
#include <optional>
#include "CrcChecker/CrcChecker.h"
#include "Decompressor/Decompressor.h"
#include "image.h"
#include "Filter/Filter.h"


inline RGB &operator+=(RGB &a, RGB other) {
    a.r += other.r;
    a.g += other.g;
    a.b += other.b;
    a.a += other.a;
    return a;
}

struct ImageWrap {
private:
    Image image;
    int x = 0;
    int y = 0;
public:
    ImageWrap(Image &im) : image(im) {}

    RGB get_next_pixel() {
        auto tmp = image(y, x);
        x++;
        if (x == image.Width()){
            y++;
            x = 0;
        }
        return tmp;
    }
};


class Decoder {
private:
    std::istream &istream;
    IHDR ihdrH{};
    std::vector<Chunk> chunks;
    std::vector<RGB> palette;
    Decompressor decompressor;

    void ValidatePngHeader() const;

    template<class T>
    T changeBytesOrder(T value) {
        auto ptr = reinterpret_cast<char *>(&value);
        uint first_byte = 0;
        auto last_byte = sizeof(T) - 1;
        while (first_byte < last_byte) {
            std::swap(ptr[first_byte++], ptr[last_byte--]);
        }
        value = *reinterpret_cast<T *>(ptr);
        return value;
    }

    template<class T>
    T extractValue(T *ptr) {
        T buf;
        std::memcpy(&buf, ptr, sizeof(T));
        return changeBytesOrder(buf);
    }

    [[nodiscard]] static bool checkAllowedCombinations(uint8_t colorType, uint8_t bitDepth) ;

    void fillInfoFromIHDR();

    void fillInfoFromPLTE();

    void ReadChunks();

    void validateCRC() const;

    std::vector<uint8_t> decompressPNG();

    [[nodiscard]] Image getImage(std::vector<uint8_t> data) const;

    [[nodiscard]] uint8_t readIthPixelFromByte(uint8_t byte, uint i) const;

    [[nodiscard]] std::vector<uint8_t> replaceData(std::vector<uint8_t> data) const;

    [[nodiscard]] RGB getCurrentPixel(const std::vector<uint8_t> &data, int offset) const;

    [[nodiscard]] int bytesPerPixel() const;

    Image getImageFrom(std::vector<uint8_t> data, int &offset, uint height, uint width) const;

    [[nodiscard]] Image mergeInterlacedImages(std::vector<ImageWrap> images) const;

public:

    explicit Decoder(std::istream &istream_) : istream(istream_) {
    }

    Image processPNG() {
        ValidatePngHeader();
        ReadChunks();
        fillInfoFromIHDR();
        fillInfoFromPLTE();
        validateCRC();

        return getImage(decompressPNG());
    }
};
