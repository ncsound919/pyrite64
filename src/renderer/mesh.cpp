/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "mesh.h"
#include "../context.h"
#include "scene.h"

void Renderer::Mesh::recreate(Renderer::Scene &scene) {
  delete vertBuff;
  vertBuff = new VertBuffer({sizeof(vertices), ctx.gpu});
  vertBuff->setData(vertices);

  scene.addOneTimeCopyPass([this](SDL_GPUCommandBuffer* cmdBuff, SDL_GPUCopyPass *copyPass){
    if (vertBuff) {
      vertBuff->upload(*copyPass);
    }
  });
}

void Renderer::Mesh::draw(SDL_GPURenderPass* pass) {
  SDL_GPUBufferBinding bufferBindings[1];
  addBinding(bufferBindings[0]);
  SDL_BindGPUVertexBuffers(pass, 0, bufferBindings, 1); // bind one buffer starting from slot 0

  SDL_DrawGPUPrimitives(pass, vertices.size(), 1, 0, 0);
}

Renderer::Mesh::Mesh() {
}

Renderer::Mesh::~Mesh() {
  delete vertBuff;
}
