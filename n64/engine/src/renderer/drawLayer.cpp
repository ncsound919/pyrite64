/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "renderer/drawLayer.h"
#include "../libdragon/rspq.h"
#include <array>
#include <vector>

#include "lib/logger.h"

namespace
{
  struct Layer
  {
    volatile uint32_t *pointer{};
    volatile uint32_t *current{};
    volatile uint32_t *sentinel{};
  };

  constexpr uint32_t LAYER_BUFFER_COUNT = 3;
  constexpr uint32_t LAYER_BUFFER_WORDS = 2048;
  std::vector<std::array<Layer, LAYER_BUFFER_COUNT>> layers{};

  constinit volatile uint32_t* layerMem{nullptr};
  constinit uint8_t frameIdx{0};
  constinit uint8_t currLayerIdx{0};
}

void P64::DrawLayer::init(uint32_t layerCount)
{
  assert(layerMem == nullptr);

  currLayerIdx = 0;
  layers = {};
  layers.resize(layerCount-1);

  size_t allocSize = layers.size() * LAYER_BUFFER_WORDS * LAYER_BUFFER_COUNT * sizeof(uint32_t);
  Log::info("DrawLayer mem-size: %d bytes", allocSize);
  layerMem = (volatile uint32_t*)malloc_uncached(allocSize);
  sys_hw_memset64((void*)layerMem, 0, allocSize);

  auto mem = layerMem;

  for(auto &layer : layers)
  {
    layer.fill({});
    for(uint32_t i = 0; i < LAYER_BUFFER_COUNT; i++)
    {
      layer[i].pointer = mem;
      layer[i].current = mem;
      layer[i].sentinel = mem + LAYER_BUFFER_WORDS;
      mem += LAYER_BUFFER_WORDS;
    }
  }
}

void P64::DrawLayer::use(uint32_t idx)
{
  if(idx == currLayerIdx)return;

  if(idx == 0)
  {
    layers[currLayerIdx-1][frameIdx].current = LD::RSPQ::redirectEnd();
  } else {
    LD::RSPQ::redirectStart(
      layers[idx-1][frameIdx].current,
      layers[idx-1][frameIdx].sentinel
    );
  }

  currLayerIdx = idx;
}

void P64::DrawLayer::drawAll()
{
  for(auto &layer : layers) {
    LD::RSPQ::exec(layer[frameIdx].pointer, layer[frameIdx].current);
  }
}

void P64::DrawLayer::draw(uint32_t layerIdx)
{
  assertf(layerIdx != 0, "Layer 0 is drawn in real-time");
  assertf(layerIdx - 1 < layers.size(), "Invalid layer index %lu", layerIdx);

  auto &layer = layers[layerIdx-1];
  LD::RSPQ::exec(layer[frameIdx].pointer, layer[frameIdx].current);
}

void P64::DrawLayer::nextFrame()
{
  frameIdx = (frameIdx + 1) % LAYER_BUFFER_COUNT;

  for(auto &layer : layers) {
    layer[frameIdx].current = layer[frameIdx].pointer;

    // @TODO: is this needed?
    sys_hw_memset64((void*)layer[frameIdx].pointer, 0, LAYER_BUFFER_WORDS * sizeof(uint32_t));
  }
}
