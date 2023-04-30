#include "Decoder.h"
#include "Filter/Filter.h"

void Decoder::ValidatePngHeader() const {
    uint8_t pngHeader[] = {137, 80, 78, 71, 13, 10, 26, 10};
    uint8_t headerBytes[8];
    istream.read(reinterpret_cast<char *>(headerBytes), 8);
    for (int i = 0; i < 8; i++) {
        if (pngHeader[i] != headerBytes[i]) {
            throw std::runtime_error("Input file is not a PNG file\n");
        }
    }
}


bool Decoder::checkAllowedCombinations(uint8_t colorType, uint8_t bitDepth) {
    std::vector<uint32_t> allowedBitDepthByColorType = {0b11111, 0, 0b1100, 0b1111, 0b1100, 0, 0b1100};

    auto allowedBitDepths = allowedBitDepthByColorType[colorType];
    if (!(allowedBitDepths & bitDepth)) {
        return false;
    }
    return true;
}

void Decoder::fillInfoFromIHDR() {
    std::vector<uint8_t> IHDR_data;
    IHDR_data = chunks[0].data;
    if (IHDR_data.empty()) throw std::runtime_error("It is wrong PNG file");
    auto ptr_to_data = IHDR_data.data();

    ihdrH.width = extractValue(reinterpret_cast<uint32_t *>(ptr_to_data));
    ptr_to_data += sizeof(ihdrH.width);
    ihdrH.height = extractValue(reinterpret_cast<uint32_t *>(ptr_to_data));
    ptr_to_data += sizeof(ihdrH.height);
    if (ihdrH.width == 0 || ihdrH.height == 0) {
        throw std::runtime_error("wrong width or height of PNG");
    }

    ihdrH.bit_depth = extractValue(reinterpret_cast<uint8_t *>(ptr_to_data));
    ptr_to_data += sizeof(ihdrH.bit_depth);
    ihdrH.color_type = extractValue(reinterpret_cast<uint8_t *>(ptr_to_data));
    ptr_to_data += sizeof(ihdrH.color_type);
    if (!checkAllowedCombinations(ihdrH.color_type, ihdrH.bit_depth)) {
        throw std::runtime_error("not allowed combination of color type and bit depth");
    }

    ihdrH.c_method = extractValue(reinterpret_cast<uint8_t *>(ptr_to_data));
    ptr_to_data += sizeof(ihdrH.c_method);
    if (ihdrH.c_method != 0) {
        throw std::runtime_error("not provided compression method");
    }

    ihdrH.f_method = extractValue(reinterpret_cast<uint8_t *>(ptr_to_data));
    ptr_to_data += sizeof(ihdrH.f_method);
    if (ihdrH.f_method != 0) {
        throw std::runtime_error("not provided filter method");
    }

    ihdrH.i_method = extractValue(reinterpret_cast<uint8_t *>(ptr_to_data));
    if (ihdrH.i_method != 0 && ihdrH.i_method != 1) {
        throw std::runtime_error("not provided interlace method");
    }
}

void Decoder::fillInfoFromPLTE() {
    for (auto chunk: chunks) {
        if (!std::memcmp("PLTE", chunk.type, sizeof(chunk.type))) {
            if (chunk.length % 3 != 0) {
                throw std::runtime_error("wrong PLTE chunk");
            }
            auto ptr_on_data = chunk.data.data();

            auto cnt_indexes = chunk.length / 3;
            while (cnt_indexes--) {
                RGB p{};
                p.r = extractValue(reinterpret_cast<uint8_t *>(ptr_on_data));
                ptr_on_data += sizeof(uint8_t);
                p.g = extractValue(reinterpret_cast<uint8_t *>(ptr_on_data));
                ptr_on_data += sizeof(uint8_t);
                p.b = extractValue(reinterpret_cast<uint8_t *>(ptr_on_data));
                ptr_on_data += sizeof(uint8_t);

                palette.push_back(p);
            }
        }
    }
}

void Decoder::ReadChunks() {
    while (istream) {
        Chunk chunk;

        istream.read(reinterpret_cast<char *>(&chunk.length), 4);
        chunk.length = changeBytesOrder(chunk.length);

        istream.read(chunk.type, 4);
        if (!std::memcmp(chunk.type, "IEND", sizeof(chunk.type))) {
            break;
        }
        chunk.data.resize(chunk.length);
        istream.read(reinterpret_cast<char *>(chunk.data.data()), chunk.length);

        istream.read(reinterpret_cast<char *>(&chunk.crc), 4);
        chunk.crc = changeBytesOrder(chunk.crc);

        chunks.push_back(chunk);
        if (istream.peek() == EOF) {
            break;
        }
    }
}

void Decoder::validateCRC() const {
    for (auto chunk: chunks) {
        std::vector<CrcData> crcData = {{chunk.type,                 sizeof(chunk.type)},
                                        {(char *) chunk.data.data(), chunk.length}};
        if (calculateCrcFromChunk(crcData) != chunk.crc) {
            throw std::runtime_error("wrong CRC from " + std::string(chunk.type) + " chunk");
        }
    }
}

std::vector<uint8_t> Decoder::decompressPNG() {
    std::vector<uint8_t> data_for_deflate;
    for (auto chunk: chunks) {
        if (!std::memcmp(chunk.type, "IDAT", sizeof(chunk.type))) {
            data_for_deflate.insert(data_for_deflate.end(), chunk.data.begin(), chunk.data.end());
        }
    }
    if (data_for_deflate.empty()) throw std::runtime_error("not provided IDAT chunk");
    return decompressor.deflate(data_for_deflate);
}


Image Decoder::mergeInterlacedImages(std::vector<ImageWrap> images) const {
    auto image = Image(ihdrH.height, ihdrH.width);
    for (uint y = 0; y < image.Height(); y++) {
        for (uint x = 0; x < image.Width(); x++) {
            if (y % 2 == 1) {
                image(y, x) = images[6].get_next_pixel();
                continue;
            }
            std::vector<std::vector<int>> pos = {{1, 6, 4, 6, 2, 6, 4, 6},
                                                 {3, 6, 4, 6, 3, 6, 4, 6}};
            auto offset_y = y % 8;
            auto offset_x = x % 8;
            if (offset_x % 2 == 1) {
                image(y, x) = images[5].get_next_pixel();
                continue;
            }
            if (offset_y == 2 || offset_y == 6) {
                image(y, x) = images[4].get_next_pixel();
                continue;
            }
            if (offset_y == 0) {
                image(y, x) = images[pos[0][offset_x] - 1].get_next_pixel();
                continue;
            }
            image(y, x) = images[pos[1][offset_x] - 1].get_next_pixel();
        }
    }
    return image;
}

[[nodiscard]] Image Decoder::getImage(std::vector<uint8_t> data) const {
    int offset = 0;
    if (ihdrH.bit_depth < 8) {
        data = replaceData(data);
    }
    if (ihdrH.i_method == 0) { // no interlace
        return getImageFrom(data, offset, ihdrH.height, ihdrH.width);
    }
    // there is interlace_method

    std::vector<std::pair<int, int>> layouts = {{7, 7},
                                                {3, 7},
                                                {3, 3},
                                                {1, 3},
                                                {0, 0},
                                                {1, 1},
                                                {0, 0}};
    std::vector<std::pair<int, int>> del = {{8, 8},
                                            {8, 8},
                                            {4, 8},
                                            {4, 4},
                                            {2, 4},
                                            {2, 2},
                                            {1, 2}};
    auto calculate_size_of_picture = [&](int i) -> std::pair<int, int> {
        auto w = (ihdrH.width + layouts[i].first) / del[i].first;
        auto h = (ihdrH.height + layouts[i].second) / del[i].second;
        return {h, w};
    };

    std::vector<ImageWrap> interlacedImages;
    for (int i = 0; i < 7; i++) {
        auto [h, w] = calculate_size_of_picture(i);
        auto im = getImageFrom(data, offset, h, w);
        auto imWrap = ImageWrap(im);
        interlacedImages.push_back(imWrap);
    }
    return mergeInterlacedImages(interlacedImages);
}

uint8_t Decoder::readIthPixelFromByte(uint8_t byte, uint i) const {
    auto size = ihdrH.bit_depth;
    uint8_t mask;
    if (size == 1) {
        mask = 1;
    } else if (size == 2) {
        mask = 0b00000011;
    } else if (size == 4) {
        mask = 0b00001111;
    } else {
        throw std::runtime_error("wrong bit depth");
    }
    return (byte >> (8 - i - size)) & mask;
}

std::vector<uint8_t> Decoder::replaceData(std::vector<uint8_t> data) const {
    int offset = 0;
    std::vector<uint8_t> new_data;
    for (uint y = 0; y < ihdrH.height; y++) {
        uint8_t filter = data[offset];
        offset++;
        new_data.push_back(filter);
        for (uint x = 0; x < ihdrH.width;) {
            auto size = ihdrH.bit_depth;
            auto cur_byte = data[offset];
            int pos = 0;
            auto cnt_bit_per_byte = 8;
            while (pos < cnt_bit_per_byte && x < ihdrH.width) {
                new_data.push_back(readIthPixelFromByte(cur_byte, pos));
                x++;
                pos += size;
            }
            offset++;
        }
    }
    return new_data;
}

RGB Decoder::getCurrentPixel(const std::vector<uint8_t> &data, int offset) const {
    if (ihdrH.color_type == 3) {
        uint8_t number = data[offset];
        return palette[number];
    }
    if (ihdrH.color_type == 0) {
        return {(uint8_t) data[offset], (uint8_t) data[offset], (uint8_t) data[offset]};
    }
    if (ihdrH.color_type == 4) {
        return {(uint8_t) data[offset], (uint8_t) data[offset], (uint8_t) data[offset], (uint8_t) data[offset + 1]};
    }
    RGB cur_pixel = {(uint8_t) data[offset], (uint8_t) data[offset + 1], (uint8_t) data[offset + 2]};
    if (ihdrH.color_type == 6) {
        cur_pixel.a = data[offset + 3];
    }
    return cur_pixel;
}


int Decoder::bytesPerPixel() const {
    int bytes_pp = ihdrH.bit_depth / 8;
    if (ihdrH.color_type == 2) {
        bytes_pp *= 3;
    } else if (ihdrH.color_type == 6) {
        bytes_pp *= 4;
    } else if (ihdrH.color_type == 4) {
        bytes_pp *= 2;
    } else {
        bytes_pp = 1; // if header.bit_depth < 8, then every byte contain pixel
        // because we replaced data
    }
    return bytes_pp;
}

Image Decoder::getImageFrom(std::vector<uint8_t> data, int &offset, uint height, uint width) const {
    auto image = Image(height, width);
    for (uint y = 0; y < image.Height(); y++) {
        int filter = data[offset];
        offset++;
        int bytes_pp = bytesPerPixel();
        for (uint x = 0; x < image.Width(); x++) {
            RGB &pix = image(y, x);
            image(y, x) = getCurrentPixel(data, offset);

            Filter::apply_filter(x, y, filter, image);
            if (ihdrH.color_type != 6 && ihdrH.color_type != 4) pix.a = 255;
            pix.r = (pix.r + 256) % 256;
            pix.g = (pix.g + 256) % 256;
            pix.b = (pix.b + 256) % 256;
            pix.a = (pix.a + 256) % 256;
            offset += bytes_pp;
        }
    }
    return image;
}