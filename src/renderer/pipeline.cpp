/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "pipeline.h"
#include "../context.h"

Renderer::Pipeline::Pipeline(const Info &info) {
  SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
  info.shader.setToPipeline(pipelineInfo);
  pipelineInfo.primitive_type = info.prim;

  // Depth
  pipelineInfo.depth_stencil_state.compare_op = SDL_GPU_COMPAREOP_LESS;
  pipelineInfo.depth_stencil_state.enable_depth_test = true;
  pipelineInfo.depth_stencil_state.enable_depth_write = true;
  pipelineInfo.target_info.has_depth_stencil_target = true;
  pipelineInfo.target_info.depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D24_UNORM_S8_UINT;

  SDL_GPUVertexBufferDescription vertBuffDesc[1];

  vertBuffDesc[0].slot = 0;
  vertBuffDesc[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
  vertBuffDesc[0].instance_step_rate = 0;
  vertBuffDesc[0].pitch = info.vertPitch;

  pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
  pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertBuffDesc;

  int idx = 0;

  std::vector<SDL_GPUVertexAttribute> vertexAttributes{};
  vertexAttributes.resize(info.vertLayout.size());

  for (auto &def : info.vertLayout) {
    vertexAttributes[idx].buffer_slot = 0;
    vertexAttributes[idx].location = idx;
    vertexAttributes[idx].format = def.format;
    vertexAttributes[idx].offset = def.offset;
    ++idx;
  }

  pipelineInfo.vertex_input_state.num_vertex_attributes = vertexAttributes.size();
  pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes.data();

  SDL_GPUColorTargetDescription colorTargetDescriptions[1] {
    {.format = SDL_GetGPUSwapchainTextureFormat(ctx.gpu, ctx.window)}
  };

  pipelineInfo.target_info.num_color_targets = 1;
  pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;

  pipeline = SDL_CreateGPUGraphicsPipeline(ctx.gpu, &pipelineInfo);
}

Renderer::Pipeline::~Pipeline() {
  SDL_ReleaseGPUGraphicsPipeline(ctx.gpu, pipeline);
}
