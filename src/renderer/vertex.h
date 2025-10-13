/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#pragma once
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"

namespace Renderer
{
  struct Vertex
  {
    glm::vec3 pos{};
    glm::vec3 norm{};
    glm::vec4 color{};
    glm::vec2 uv{};
  };
}
