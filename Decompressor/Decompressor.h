#pragma once

#include <libdeflate.h>
#include <vector>
#include <iostream>

struct Decompressor {
    struct libdeflate_decompressor *decompressor;
public:
    Decompressor() : decompressor(libdeflate_alloc_decompressor()) {}

    Decompressor(const Decompressor &) = delete;

    Decompressor(Decompressor &&) = delete;

    Decompressor &operator=(const Decompressor &) = delete;

    Decompressor &operator=(Decompressor &&) = delete;

    [[nodiscard]] std::vector<uint8_t> deflate(const std::vector<uint8_t> &data) const {
        std::vector<uint8_t> output(data.size() * 4);
        size_t actual_out_n_bytes_ret;
        repeat:
        auto res = libdeflate_zlib_decompress(decompressor, data.data(), data.size(), output.data(), output.size(),
                                              &actual_out_n_bytes_ret);
        if (res == libdeflate_result::LIBDEFLATE_INSUFFICIENT_SPACE) {
            std::cerr << "insuffient space, buffer resized ...\n";
            output.resize(2 * output.size());
            goto repeat;
        } else if (res != libdeflate_result::LIBDEFLATE_SUCCESS) {
            throw std::runtime_error("decompressing is unsuccessful");
        }
        output.resize(actual_out_n_bytes_ret);
        return output;
    }

    ~Decompressor() {
        libdeflate_free_decompressor(decompressor);
    }
};
