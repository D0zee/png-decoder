# PNG-decoder

## Overview

This project is a PNG image decoder that supports grayscale images, regular RGB, indexed images with a fixed palette, and alpha-channel transparency. It decodes PNG images using a pipeline of steps that involves reading the signature bytes, parsing chunks until the IEND chunk is found, validating the CRC of each chunk, concatenating the content of all IDAT chunks into a single byte vector, and decoding the byte vector using deflate. The decoded data is processed by scanlines, applying specified filters.

This decoder supports all possible combinations of modes in IHDR, with bit depth <= 8. 

## Usage

To use this decoder, include `png_decoder.h` in your project and call the `ReadPng` function with the filename of the PNG image as the argument. The function returns an `Image` object, which contains the width, height, pixel format, and pixel data of the decoded image.

## Dependencies

- https://github.com/ebiggers/libdeflate (for deflating data)
- boost (for CRC)

## Supported features

- Grayscale images
- Regular RGB images
- Indexed images with a fixed palette
- Alpha-channel transparency
- Interlaced images

## Unsupported features
- bit depth = 16
- Ancillary chunks (except for IHDR and PLTE chunks)