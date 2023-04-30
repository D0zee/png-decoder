

set(PNG_DECODER_SOURCES
        Decoder/Decoder.cpp
        CrcChecker/CrcChecker.cpp
        Filter/Filter.cpp
        )

add_library(png_decoder_lib ${PNG_DECODER_SOURCES})

find_library(DEFLATE NAMES libdeflate.a)
find_package(Boost COMPONENTS system REQUIRED)

target_link_libraries(png_decoder_lib "${DEFLATE}")
target_link_libraries(png_decoder_lib Boost::system)

target_include_directories(png_decoder_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set(PNG_STATIC png_decoder_lib)




