/**
* @copyright 2025 - Max Bebök
* @license MIT
*/
#include "projectBuilder.h"
#include "../utils/string.h"
#include <filesystem>
#include <format>

#include "../utils/fs.h"
#include "../utils/logger.h"

namespace fs = std::filesystem;

void Build::buildScripts(Project::Project &project, SceneCtx &sceneCtx)
{
  auto pathTable = project.getPath() + "/src/p64/scriptTable.cpp";

  std::string srcEntries = "";
  std::string srcSizeEntries = "";
  std::string srcDecl = "";

  auto scripts = project.getAssets().getTypeEntries(Project::AssetManager::FileType::CODE);
  uint32_t idx = 0;
  for (auto &script : scripts)
  {
    auto src = Utils::FS::loadTextFile(script.path);
    bool hasInit = Utils::CPP::hasFunction(src, "void", "init");
    bool hasUpdate = Utils::CPP::hasFunction(src, "void", "update");
    bool hasDraw = Utils::CPP::hasFunction(src, "void", "draw");
    bool hasDestroy = Utils::CPP::hasFunction(src, "void", "destroy");

    auto uuidStr = std::format("{:016X}", script.uuid);

    srcSizeEntries += uuidStr + "::DATA_SIZE,\n";

    srcDecl += "  namespace " + uuidStr + " {\nstruct Data;\n";
    srcDecl += " extern uint16_t DATA_SIZE;\n";
    if(hasInit)srcDecl += "void init(Object& obj, Data *data);\n";
    if(hasUpdate)srcDecl += "void update(Object& obj, Data *data);\n";
    if(hasDraw)srcDecl += "void draw(Object& obj, Data *data);\n";
    if(hasDestroy)srcDecl += "void destroy(Object& obj, Data *data);\n";
    srcDecl += "}\n";

    srcEntries += "{\n";
    if(hasInit)srcEntries += " .init = (FuncObject)" + uuidStr + "::init,\n";
    if(hasUpdate)srcEntries += " .update = (FuncObject)" + uuidStr + "::update,\n";
    if(hasDraw)srcEntries += " .draw = (FuncObject)" + uuidStr + "::draw,\n";
    if(hasDestroy)srcEntries += " .destroy = (FuncObject)" + uuidStr + "::destroy,\n";
    srcEntries += "},\n";

    sceneCtx.codeIdxMapUUID[script.uuid] = idx;

    Utils::Logger::log("Script: " + uuidStr + " -> " + std::to_string(idx));
    ++idx;
  }

  auto src = Utils::FS::loadTextFile("data/scripts/scriptTable.cpp");
  src = Utils::replaceAll(src, "__CODE_ENTRIES__", srcEntries);
  src = Utils::replaceAll(src, "__CODE_SIZE_ENTRIES__", srcSizeEntries);
  src = Utils::replaceAll(src, "__CODE_DECL__", srcDecl);


  Utils::FS::saveTextFile(pathTable, src);
}