#pragma once

#include <iostream>
#include <vector>

struct RGB {
    uint r = 0, g = 0, b = 0, a = 0;
    bool operator==(const RGB& rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
    }
};

inline std::ostream& operator<<(std::ostream& out, const RGB& x) {
    out << x.r << " " << x.g << " " << x.b << " " << x.a;
    return out;
}

class Image {
public:
    Image() = default;
    Image(uint height, uint width) {
        SetSize(height, width);
    }

    void SetSize(uint height, uint width) {
        height_ = height;
        width_ = width;
        data_.resize(height_ * width_);
    }

    const RGB& operator()(uint row, uint col) const {
        return data_[width_ * row + col];
    }

    RGB& operator()(uint row, uint col) {
        return data_[width_ * row + col];
    }

    [[nodiscard]] uint Height() const {
        return height_;
    }

    [[nodiscard]] uint Width() const {
        return width_;
    }
private:
    std::vector<RGB> data_;
    uint height_ = 0;
    uint width_ = 0;
};

