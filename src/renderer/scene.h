/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include <functional>
#include <SDL3/SDL.h>

namespace Renderer
{
  using CbRenderPass = std::function<void(SDL_GPUCommandBuffer*, SDL_GPUGraphicsPipeline*)>;
  using CbCopyPass = std::function<void(SDL_GPUCommandBuffer*, SDL_GPUCopyPass*)>;

  class Scene
  {
    private:
      std::unordered_map<uint32_t, CbRenderPass> renderPasses{};
      std::unordered_map<uint32_t, CbCopyPass> copyPasses{};
      std::vector<CbCopyPass> copyPassesOneTime{};

    public:
      Scene();
      ~Scene();

      void update();
      void draw();

      void addRenderPass(uint32_t id, const CbRenderPass& pass) { renderPasses[id] = pass; }
      void removeRenderPass(uint32_t id) { renderPasses.erase(id); }

      void addCopyPass(uint32_t id, const CbCopyPass& pass) { copyPasses[id] = pass; }
      void removeCopyPass(uint32_t id) { copyPasses.erase(id); }

      void addOneTimeCopyPass(const CbCopyPass& pass) { copyPassesOneTime.push_back(pass); }
  };
}
