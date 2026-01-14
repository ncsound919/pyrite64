/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "../components.h"
#include "../../../context.h"
#include "../../../editor/imgui/helper.h"
#include "../../../utils/json.h"
#include "../../../utils/jsonBuilder.h"
#include "../../../utils/binaryFile.h"
#include "../../../utils/logger.h"
#include "../../assetManager.h"
#include "../../../editor/pages/parts/viewport3D.h"
#include "../../../renderer/scene.h"
#include "../../../utils/meshGen.h"
#include "../../../shader/defines.h"
#include "../shared/material.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"

namespace Project::Component::Model
{
  struct Data
  {
    PROP_U64(model);
    PROP_S32(layerIdx);
    PROP_BOOL(culling);
    Shared::Material material{};

    Renderer::Object obj3D{};
    Utils::AABB aabb{};
  };

  std::shared_ptr<void> init(Object &obj) {
    return std::make_shared<Data>();
  }

  nlohmann::json serialize(const Entry &entry)
  {
    Data &data = *static_cast<Data*>(entry.data.get());
    return Utils::JSON::Builder{}
      .set(data.model)
      .set(data.layerIdx)
      .set(data.culling)
      /*.set(data.matSetDepthRead)
      .set(data.matDepthRead)
      .set(data.matSetDepthWrite)
      .set(data.matDepthWrite)*/
      .doc;
  }

  std::shared_ptr<void> deserialize(nlohmann::json &doc) {
    auto data = std::make_shared<Data>();
    Utils::JSON::readProp(doc, data->layerIdx);
    Utils::JSON::readProp(doc, data->model);
    Utils::JSON::readProp(doc, data->culling, false);
    /*Utils::JSON::readProp(doc, data->matSetDepthRead, false);
    Utils::JSON::readProp(doc, data->matDepthRead, false);
    Utils::JSON::readProp(doc, data->matSetDepthWrite, false);
    Utils::JSON::readProp(doc, data->matDepthWrite, false);*/
    return data;
  }

  void build(Object& obj, Entry &entry, Build::SceneCtx &ctx)
  {
    Data &data = *static_cast<Data*>(entry.data.get());

    auto res = ctx.assetUUIDToIdx.find(data.model.value);
    uint16_t id = 0xDEAD;
    if (res == ctx.assetUUIDToIdx.end()) {
      Utils::Logger::log("Component Model: Model UUID not found: " + std::to_string(entry.uuid), Utils::Logger::LEVEL_ERROR);
    } else {
      id = res->second;
    }

    ctx.fileObj.write<uint16_t>(id);
    ctx.fileObj.write<uint8_t>(data.layerIdx.value);
    ctx.fileObj.write<uint8_t>(data.culling.resolve(obj.propOverrides));
  }

  void draw(Object &obj, Entry &entry)
  {
    Data &data = *static_cast<Data*>(entry.data.get());

    auto &assets = ctx.project->getAssets();
    auto &modelList = assets.getTypeEntries(AssetManager::FileType::MODEL_3D);
    auto scene = ctx.project->getScenes().getLoadedScene();

    if (ImTable::start("Comp", &obj)) {
      ImTable::add("Name", entry.name);
      ImTable::add("Model");

      if (ImGui::VectorComboBox("Model", modelList, data.model.value)) {
        data.obj3D.removeMesh();
      }

      std::vector<const char*> layerNames{};
      for (auto &layer : scene->conf.layers3D) {
        layerNames.push_back(layer.name.value.c_str());
      }
      ImTable::addComboBox("Draw-Layer", data.layerIdx.resolve(obj.propOverrides), layerNames);

      ImTable::addObjProp("Culling", data.culling);

      if(data.culling.resolve(obj.propOverrides)) {
        auto modelAsset = ctx.project->getAssets().getEntryByUUID(data.model.value);
        if(modelAsset && !modelAsset->conf.gltfBVH) {
          ImGui::SameLine();
          ImGui::TextColored({1.0f, 0.5f, 0.5f, 1.0f}, "Warning: BVH not enabled!");
        }
      }

      ImTable::end();

      // disable background
      ImGui::PushStyleColor(ImGuiCol_Header, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
      ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
      ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));

      if(ImGui::CollapsingHeader("Material Settings", ImGuiTreeNodeFlags_DefaultOpen))
      {

      }

      ImGui::PopStyleColor(3);





      /*if(data.matOverride.resolve(obj.propOverrides))
      {
        auto asset = ctx.project->getAssets().getEntryByUUID(data.model.value);
        if(asset)
        {
          for(const auto &model : asset->t3dmData.models)
          {
            ImGui::Text("Model: %s", model.name.c_str());
          }
        }
      }*/


    }
  }

  void draw3D(Object& obj, Entry &entry, Editor::Viewport3D &vp, SDL_GPUCommandBuffer* cmdBuff, SDL_GPURenderPass* pass)
  {
    Data &data = *static_cast<Data*>(entry.data.get());
    if (!data.obj3D.isMeshLoaded()) {
      auto asset = ctx.project->getAssets().getEntryByUUID(data.model.value);
      if (asset && asset->mesh3D) {
        if (!asset->mesh3D->isLoaded()) {
          asset->mesh3D->recreate(*ctx.scene);
        }
        data.aabb = asset->mesh3D->getAABB();
        data.obj3D.setMesh(asset->mesh3D);
      }
    }

    if(ctx.project->getScenes().getLoadedScene()->conf.renderPipeline.value == 2)
    {
      data.obj3D.uniform.mat.flags = 0;
      if(data.layerIdx.value == 0)data.obj3D.uniform.mat.flags |= T3D_FLAG_NO_LIGHT;
    }

    data.obj3D.setObjectID(obj.uuid);

    // @TODO: tidy-up
    glm::vec3 skew{0,0,0};
    glm::vec4 persp{0,0,0,1};
    data.obj3D.uniform.modelMat = glm::recompose(
      obj.scale.resolve(obj.propOverrides),
      obj.rot.resolve(obj.propOverrides),
      obj.pos.resolve(obj.propOverrides),
      skew, persp);

    data.obj3D.draw(pass, cmdBuff);

    bool isSelected = ctx.selObjectUUID == obj.uuid;
    if (isSelected)
    {
      auto center = obj.pos.resolve(obj.propOverrides) + (data.aabb.getCenter() * obj.scale.resolve(obj.propOverrides) * (float)0xFFFF);
      auto halfExt = data.aabb.getHalfExtend() * obj.scale.resolve(obj.propOverrides) * (float)0xFFFF;

      glm::u8vec4 aabbCol{0xAA,0xAA,0xAA,0xFF};
      if (isSelected) {
        aabbCol = {0xFF,0xAA,0x00,0xFF};
      }

      Utils::Mesh::addLineBox(*vp.getLines(), center, halfExt, aabbCol);
      Utils::Mesh::addLineBox(*vp.getLines(), center, halfExt + 0.002f, aabbCol);
    }
  }
}
