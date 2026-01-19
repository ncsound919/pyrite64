/**
* @copyright 2026 - Max Bebök
* @license MIT
*/
#include "script/nodeGraph.h"

#include "scene/object.h"

namespace P64::NodeGraph
{
  struct NodeDef
  {
    uint8_t type{};
    uint8_t outCount{};
    uint16_t outOffsets[];

    NodeDef *getNext(uint32_t idx) {
      if(idx >= outCount)return nullptr;
      return (NodeDef*)((uint8_t*)this + outOffsets[idx]);
    }

    uint16_t *getDataPtr() {
      return (uint16_t*)&outOffsets[outCount];
    }
  };

  struct GraphDef
  {
    uint16_t nodeCount;
    NodeDef start;
  };

  void printNode(NodeDef* node, int level)
  {
    debugf("%*s", level * 2, "");
    debugf("%p Type:%d, outputs: %d ", node, node->type, node->outCount);
    for (uint16_t i = 0; i < node->outCount; i++) {
      debugf("0x%04X ", node->outOffsets[i]);
    }

    uint16_t *nodeData = (uint16_t*)&node->outOffsets[node->outCount];
    //debugf(", data: %04X", *nodeData);
    debugf("\n");

    for (uint16_t i = 0; i < node->outCount; i++) {
      auto nextNode = (NodeDef*)((uint8_t*)node + node->outOffsets[i]);
      printNode(nextNode, level + 1);
    }
  };

}

void P64::NodeGraph::Instance::update(float deltaTime) {
  if(!currNode)return;

  printNode(currNode, 0);
  uint16_t *data = currNode->getDataPtr();

  debugf("reg: %d ", reg);
  switch(currNode->type)
  {
    case 0:
      reg += (uint16_t)(deltaTime * 1000.0f);
      if(reg < data[0])return;
      reg = 0;
      break;
    case 1:
      if(object)object->remove();
      break;
  }

  currNode = currNode->getNext(0);
}
