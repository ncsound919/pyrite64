/**
* @copyright 2024 - Max Bebök
* @license MIT
*/
#include "collision/bvh.h"
// #include "../debug/debugDraw.h"

namespace {
  const int16_t *ctxData;
  const P64::Coll::AABB *ctxAABB;
  const P64::Coll::IVec3 *ctxRayPos;
  P64::Coll::BVHResult *ctxRes;

  void queryNodeAABB(const P64::Coll::BVHNode *node)
  {
    if(!node->aabb.vsAABB(*ctxAABB))return;

    int dataCount = node->value & 0b1111;
    int offset = (int16_t)node->value >> 4;

    if(dataCount == 0) {
      queryNodeAABB(&node[offset]);
      queryNodeAABB(&node[offset + 1]);
      return;
    }

    int offsetEnd = offset + dataCount;
    while(offset < offsetEnd && ctxRes->count < P64::Coll::MAX_RESULT_COUNT) {
      ctxRes->triIndex[ctxRes->count++] = ctxData[offset++];
    }
  }

  void queryNodeRaycastFloor(const P64::Coll::BVHNode *node)
  {
    if(!node->aabb.vs2DPointY(*ctxRayPos))return;

    int dataCount = node->value & 0b1111;
    int offset = (int16_t)node->value >> 4;

    if(dataCount == 0) {
      queryNodeRaycastFloor(&node[offset]);
      queryNodeRaycastFloor(&node[offset + 1]);
      return;
    }

    int offsetEnd = offset + dataCount;
    while(offset < offsetEnd && ctxRes->count < P64::Coll::MAX_RESULT_COUNT) {
      ctxRes->triIndex[ctxRes->count++] = ctxData[offset++];
    }
  }
}

void P64::Coll::BVH::vsAABB(const AABB &aabb, BVHResult &res) const {
  ctxData = (int16_t*)&nodes[nodeCount]; // data starts right after nodes;
  ctxAABB = &aabb;
  ctxRes = &res;
  queryNodeAABB(nodes);
}

void P64::Coll::BVH::raycastFloor(const IVec3 &pos, BVHResult &res) const {
  ctxData = (int16_t*)&nodes[nodeCount]; // data starts right after nodes;
  ctxRayPos = &pos;
  ctxRes = &res;
  queryNodeRaycastFloor(nodes);
}


