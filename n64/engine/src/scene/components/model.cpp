/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "assets/assetManager.h"
#include "scene/object.h"
#include "scene/components/model.h"
#include "assets/assetManager.h"
#include <t3d/t3dmodel.h>

#include "../../renderer/bigtex/bigtex.h"
#include "renderer/material.h"
#include "scene/scene.h"
#include "scene/sceneManager.h"

namespace
{
  struct InitData
  {
    uint16_t assetIdx;
    uint8_t layer;
    uint8_t flags;
    P64::Renderer::Material material;
  };

  void drawModel(T3DModel *model)
  {
    T3DModelState state = t3d_model_state_create();
    state.drawConf = nullptr;
    state.lastBlendMode = 0;

    T3DModelIter it = t3d_model_iter_create(model, T3D_CHUNK_TYPE_OBJECT);
    while(t3d_model_iter_next(&it))
    {
      it.object->material->blendMode = 0;
      t3d_model_draw_material(it.object->material, &state);
      t3d_model_draw_object(it.object, nullptr);
    }

    if(state.lastVertFXFunc != T3D_VERTEX_FX_NONE)t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0, 0);
  }
}

namespace P64::Comp
{
  void Model::initDelete([[maybe_unused]] Object& obj, Model* data, void* initData_)
  {
    auto *initData = (InitData*)initData_;
    if (initData == nullptr) {
      data->~Model();
      return;
    }

    new(data) Model();

    data->model = (T3DModel*)AssetManager::getByIndex(initData->assetIdx);
    assert(data->model != nullptr);
    data->layerIdx = initData->layer;
    data->flags = initData->flags;
    data->material = initData->material;

    bool isBigTex = SceneManager::getCurrent().getConf().pipeline == SceneConf::Pipeline::BIG_TEX_256;
    bool separate = data->flags & FLAG_CULLING;

    if(isBigTex) {
      Renderer::BigTex::patchT3DM(*data->model);
      return;
    }

    if(separate)
    {
      auto it = t3d_model_iter_create(data->model, T3D_CHUNK_TYPE_OBJECT);
      while(t3d_model_iter_next(&it)) {
        if(it.object->userBlock)return; // already recorded the model
        rspq_block_begin();
          t3d_model_draw_material(it.object->material, nullptr);
          t3d_model_draw_object(it.object, nullptr);

          if(it.object->material->vertexFxFunc) { // @TODO: fix this in t3d
            t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0,0);
          }
        it.object->userBlock = rspq_block_end();
      }
      //t3d_state_set_vertex_fx(T3D_VERTEX_FX_NONE, 0,0);
    } else {
      if(data->model->userBlock)return; // already recorded the model
      rspq_block_begin();
        drawModel(data->model);
      data->model->userBlock = rspq_block_end();
    }
  }

  void Model::draw(Object &obj, Model* data, float deltaTime)
  {
    auto mat = data->matFP.getNext();
    t3d_mat4fp_from_srt(mat, obj.scale, obj.rot, obj.pos);

    if(data->layerIdx)DrawLayer::use3D(data->layerIdx);

    data->material.begin();

    t3d_matrix_set(mat, true);

    if(data->flags & FLAG_CULLING)
    {
      auto frustum = t3d_viewport_get()->viewFrustum;
      t3d_frustum_scale(&frustum, obj.scale.x); // @TODO: handle non-uniform scale

      const T3DBvh *bvh = t3d_model_bvh_get(data->model);
      assert(bvh);
      t3d_model_bvh_query_frustum(bvh, &frustum);

      auto it = t3d_model_iter_create(data->model, T3D_CHUNK_TYPE_OBJECT);
      while(t3d_model_iter_next(&it)) {
        if(it.object->isVisible) {
          rspq_block_run(it.object->userBlock);
          it.object->isVisible = false;
        }
      }
    } else {
      rspq_block_run(data->model->userBlock);
    }

    data->material.end();
    if(data->layerIdx)DrawLayer::useDefault();
  }
}
