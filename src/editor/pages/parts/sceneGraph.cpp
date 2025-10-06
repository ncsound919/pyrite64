/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "sceneGraph.h"

#include "imgui.h"
#include "../../../context.h"
#include "IconsFontAwesome4.h"

namespace
{
  Project::Object* deleteObj{nullptr};

  void drawObjectNode(Project::Scene &scene, Project::Object &obj)
  {
    ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow
      | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DrawLinesFull;

    if (obj.children.empty()) {
      flag |= ImGuiTreeNodeFlags_Leaf;
    }

    bool isSelected = ctx.selObjectUUID == obj.uuid;
    if (isSelected) {
      flag |= ImGuiTreeNodeFlags_Selected;
    }

    auto nameID = obj.name + "##" + std::to_string(obj.uuid);
    if(ImGui::TreeNodeEx(nameID.c_str(), flag))
    {
      if (obj.parent && (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace))) {
        deleteObj = &obj;
      }

      if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        ctx.selObjectUUID = obj.uuid;
        ImGui::SetWindowFocus("Object");
        //ImGui::SetWindowFocus("Graph");
      }
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        ctx.selObjectUUID = obj.uuid;
        ImGui::OpenPopup("NodePopup");
      }

      if (ImGui::BeginPopupContextItem("NodePopup"))
      {
        if (ImGui::MenuItem(ICON_FA_CUBE " Add Empty")) {
          ctx.selObjectUUID = scene.addObject(obj)->uuid;
        }

        if (obj.parent) {
          ImGui::Separator();
          if (ImGui::MenuItem(ICON_FA_TRASH " Delete"))deleteObj = &obj;
        }
        ImGui::EndPopup();
      }

      for(auto &child : obj.children) {
        drawObjectNode(scene, *child);
      }

      ImGui::TreePop();
    }
  }
}

void Editor::SceneGraph::draw()
{
  auto scene = ctx.project->getScenes().getLoadedScene();
  if (!scene)return;

  // Menu
  if(ImGui::BeginMenuBar())
  {
    if(ImGui::BeginMenu(ICON_FA_PLUS)) {
      if(ImGui::MenuItem("Empty")){}
      if(ImGui::MenuItem("Group")){}
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }

  // Graph
  //style.IndentSpacing
  ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 18.0f);

  auto &root = scene->getRootObject();
  drawObjectNode(*scene, root);

  ImGui::PopStyleVar();

  if (deleteObj) {
    scene->removeObject(*deleteObj);
    deleteObj = nullptr;
  }
}
