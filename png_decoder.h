#pragma once

#include "image.h"
#include "Decoder/Decoder.h"
#include "Decoder/chunks.h"
#include <string>
#include <fstream>



inline Image ReadPng(std::string_view filename) {
    std::ifstream input_file(filename.data(), std::ios::binary);
    if (!input_file) {
        std::cerr << "Failed to open input file " << filename << "\n";
        exit(1);
    }
    auto decoder = Decoder(input_file);

    return decoder.processPNG();
}