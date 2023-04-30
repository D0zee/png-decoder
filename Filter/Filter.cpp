#include "Filter.h"
#include "Decoder/Decoder.h"

uint8_t Filter::PaethPredictor(uint8_t a, uint8_t b, uint8_t c) {
    int p = a + b - c;
    auto pa = abs(p - a);
    auto pb = abs(p - b);
    auto pc = abs(p - c);
    if (pa <= pb && pa <= pc) return a;
    else if (pb <= pc) return b;
    return c;
}

void Filter::PaethFilter(Image &image, uint x, uint y) {
    auto a_pix = (x == 0) ? RGB{} : image(y, x - 1);
    auto b_pix = (y == 0) ? RGB{} : image(y - 1, x);
    auto c_pix = (x == 0 || y == 0) ? RGB{} : image(y - 1, x - 1);
    image(y, x) += {PaethPredictor(a_pix.r, b_pix.r, c_pix.r),
                    PaethPredictor(a_pix.g, b_pix.g, c_pix.g),
                    PaethPredictor(a_pix.b, b_pix.b, c_pix.b),
                    PaethPredictor(a_pix.a, b_pix.a, c_pix.a)};
}

void Filter::SubFilter(Image &image, uint x, uint y) {
    auto a_pix = (x == 0) ? RGB{} : image(y, x - 1);
    image(y, x) += a_pix;
}

void Filter::UpFilter(Image &image, uint x, uint y) {
    auto b_pix = (y == 0) ? RGB{} : image(y - 1, x);
    image(y, x) += b_pix;
}

void Filter::AverageFilter(Image &image, uint x, uint y) {
    auto a_pix = (x == 0) ? RGB{} : image(y, x - 1);
    auto b_pix = (y == 0) ? RGB{} : image(y - 1, x);
    auto func = [](uint8_t a, uint8_t b) -> uint8_t {
        return floor((a + b) / 2);
    };

    image(y, x) += {func(a_pix.r, b_pix.r),
                    func(a_pix.g, b_pix.g),
                    func(a_pix.b, b_pix.b),
                    func(a_pix.a, b_pix.a),};
}

void Filter::apply_filter(uint x, uint y, int filter, Image &image) {
    if (filter == 1) {
        SubFilter(image, x, y);
    } else if (filter == 2) {
        UpFilter(image, x, y);
    } else if (filter == 3) {
        AverageFilter(image, x, y);
    } else if (filter == 4) {
        PaethFilter(image, x, y);
    }
}