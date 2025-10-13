/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include "glm/mat4x4.hpp"

namespace Renderer
{
  struct UniformGlobal
  {
    glm::mat4 projMat{};
    glm::mat4 cameraMat{};
  };

  struct UniformsObject
  {
    glm::mat4 modelMat{};
  };
}
