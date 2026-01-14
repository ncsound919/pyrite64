/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "sceneInspector.h"
#include "../../../context.h"
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "../../imgui/helper.h"

Editor::SceneInspector::SceneInspector() {
}

void Editor::SceneInspector::draw() {
  auto scene = ctx.project->getScenes().getLoadedScene();
  if(!scene)return;

  if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImTable::start("Settings");
    ImTable::addProp("Name", scene->conf.name);

    constexpr const char* OPTIONS[] = {"Default", "HDR-Bloom", "HiRes-Tex (256x)"};
    ImTable::addComboBox(
      "Pipeline",
      scene->conf.renderPipeline.value,
      OPTIONS, 3
    );

    ImTable::end();
  }

  bool fbDisabled = false;
  if(scene->conf.renderPipeline.value != 0)
  {
    // HDR/Bloom and bigtex both need those specific settings to work:
    scene->conf.fbWidth = 320;
    scene->conf.fbHeight = 240;
    scene->conf.fbFormat = 0;
    scene->conf.clearColor.value = {0,0,0,0};
    fbDisabled = true;
  }

  if (ImGui::CollapsingHeader("Framebuffer", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImTable::start("Framebuffer");

    if(fbDisabled)ImGui::BeginDisabled();
    ImTable::add("Width", scene->conf.fbWidth);
    ImTable::add("Height", scene->conf.fbHeight);

    constexpr const char* const FORMATS[] = {"RGBA16","RGBA32"};
    ImTable::addComboBox("Format", scene->conf.fbFormat, FORMATS, 2);

    ImTable::addColor("Color", scene->conf.clearColor.value, false);
    scene->conf.clearColor.value.a = 1.0f;

    ImTable::addProp("Clear Color", scene->conf.doClearColor);

    if(fbDisabled)ImGui::EndDisabled();

    ImTable::addProp("Clear Depth", scene->conf.doClearDepth);
    ImTable::end();
  }
}
