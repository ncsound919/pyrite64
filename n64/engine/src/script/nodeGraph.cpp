/**
* @copyright 2026 - Max Bebök
* @license MIT
*/
#include "script/nodeGraph.h"

#include <unordered_set>

#include "scene/object.h"
#include "scene/scene.h"
#include "script/scriptTable.h"

namespace
{
  std::unordered_map<uint32_t, P64::NodeGraph::UserFunc> userFunctionMap{};
}

namespace P64::NodeGraph
{
  struct GraphDef
  {
    GraphFunc func;
    uint32_t _padding;
    uint16_t stackSize;
  };

  void* load(const char* path)
  {
    auto data = asset_load(path, nullptr);
    uint64_t uuid = ((uint64_t*)data)[0];
    // debugf("Loaded NodeGraph: %s (UUID: %016llX)\n", path, uuid);
    ((GraphFunc*)data)[0] = P64::Script::getGraphFuncByUUID(uuid);
    return data;
  }
}

void P64::NodeGraph::Instance::load(uint16_t assetIdx)
{
  graphDef = (GraphDef*)AssetManager::getByIndex(assetIdx);
  debugf("Stack-size: %d %d\n", assetIdx, graphDef->stackSize);
  corot = corot_create(graphDef->func, this, graphDef->stackSize*2);
}

P64::NodeGraph::Instance::~Instance()
{
  if(corot) {
    corot_destroy(corot);
    corot = nullptr;
  }
}

void P64::NodeGraph::Instance::update(float deltaTime) {
  //debugf("Instance::update: %p\n", corot);
  if(!corot)return;

  //auto t = get_ticks();
  //disable_interrupts();
  corot_resume(corot);
  //enable_interrupts();
  //t = get_ticks() - t;

  if(corot_finished(corot))
  {
    corot_destroy(corot);
    corot = nullptr;
  }
}

void P64::NodeGraph::registerFunction(uint32_t strCRC32, UserFunc fn)
{
  userFunctionMap[strCRC32] = fn;
}

P64::NodeGraph::UserFunc P64::NodeGraph::getFunction(uint64_t uuid)
{
  auto it = userFunctionMap.find((uint32_t)uuid);
  if(it != userFunctionMap.end()) {
    return it->second;
  }
  return nullptr;
}
