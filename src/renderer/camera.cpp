/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "camera.h"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
//#include "glm/gtx/quaternion.hpp"

void Renderer::Camera::update() {
}

void Renderer::Camera::apply(UniformGlobal &uniGlobal)
{
  float x = SDL_GetTicks() / 1000.0f;
  uniGlobal.projMat = glm::perspective(80.0f, 1.0f, 0.1f, 100.0f);

  //uniGlobal.cameraMat = glm::lookAt(glm::vec3{0,0,-10}, {sinf(x) * 4,0,0}, {0,1,0});

  uniGlobal.cameraMat = glm::mat4_cast(rot);
}

