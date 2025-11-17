/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include <libdragon.h>
#include <type_traits>

#define P64_DATA(...) struct Data { __VA_ARGS__ }; \
  static_assert(sizeof(Data) < 0xFFFF); \
  constinit uint16_t DATA_SIZE = static_cast<uint16_t>( \
    std::is_empty_v<Data> ? 0 : sizeof(Data) \
  );
