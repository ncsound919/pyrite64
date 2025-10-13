/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include <SDL3/SDL.h>

#include "uniforms.h"
#include "glm/vec3.hpp"
#include "glm/gtc/quaternion.hpp"

namespace Renderer
{
  class Camera
  {
    private:

    public:
      glm::vec3 pos{};
      glm::quat rot{};

      void update();

      void apply(UniformGlobal &uniGlobal);
  };
}
