#pragma once

#include "image.h"
#include <cmath>

class Filter {
private:
    static uint8_t PaethPredictor(uint8_t a, uint8_t b, uint8_t c);

    static void PaethFilter(Image &image, uint x, uint y);

    static void SubFilter(Image &image, uint x, uint y);

    static void UpFilter(Image &image, uint x, uint y);

    static void AverageFilter(Image &image, uint x, uint y);

public:
    static void apply_filter(uint x, uint y, int filter, Image &image);
};