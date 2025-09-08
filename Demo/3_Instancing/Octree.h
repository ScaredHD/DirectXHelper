#pragma once

#include <vector>

#include "Culling.h"



template<typename ObjectType>
struct OctreeNode {
  AABB bbox;
  OctreeNode* children[8];
  std::vector<ObjectType*> objects;

  bool IsLeaf() const { return children[0] == nullptr; }
};
