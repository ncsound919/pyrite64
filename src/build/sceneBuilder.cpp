/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "projectBuilder.h"
#include "../utils/string.h"
#include <filesystem>

#include "../utils/binaryFile.h"

namespace fs = std::filesystem;

void Build::buildScene(Project::Project &project, const Project::SceneEntry &scene)
{
  printf(" - Scene %d: %s\n", scene.id, scene.name.c_str());
  std::string fileName = "s" + Utils::padLeft(std::to_string(scene.id), '0', 4);

  std::unique_ptr<Project::Scene> sc{new Project::Scene(scene.id)};

  auto fsDataPath = fs::absolute(fs::path{project.getPath()} / "filesystem" / "p64");

  Utils::BinaryFile sceneFile{};
  sceneFile.write<uint16_t>(sc->conf.fbWidth);
  sceneFile.write<uint16_t>(sc->conf.fbHeight);
  //sceneFile.write<uint32_t>(flags);
  //sceneFile.writeColor(scene.clearColor);
  //sceneFile.write<uint32_t>(scene.objects.length);

  sceneFile.writeToFile(fsDataPath / fileName);
}
