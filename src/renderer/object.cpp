/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "object.h"

#include "glm/ext/matrix_transform.hpp"

void Renderer::Object::draw(SDL_GPURenderPass* pass, SDL_GPUCommandBuffer* cmdBuff) {
  if (!mesh) return;

  if (transformDirty) {
    auto m = glm::identity<glm::mat4>();
    m = glm::scale(m, {0.1f, 0.1f, 0.1f});
    m = glm::translate(m, pos);
    uniform.modelMat = m;
    transformDirty = false;
  }

  SDL_PushGPUVertexUniformData(cmdBuff, 1, &uniform, sizeof(uniform));
  mesh->draw(pass);
}
