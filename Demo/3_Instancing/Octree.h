#pragma once

#include "Culling.h"
#include "PCH.h"

#undef min
#undef max

bool ShouldSubdivide(float boxSize, size_t objectCount, int depth);

std::vector<AABB> SubdivideBox(const AABB& box);


template<typename ObjectType>
struct OctreeNode {
  AABB bbox;
  OctreeNode* children[8]{};
  std::vector<ObjectType*> objects{};
  OctreeNode* parent{};
  int depth;

  OctreeNode() = default;
  explicit OctreeNode(const AABB& box, int depth, float looseFactor = 1.f) : depth{depth}
  {
    using namespace DirectX;
    auto xmMin = XMLoadFloat3(&box.min);
    auto xmMax = XMLoadFloat3(&box.max);
    auto xmCenter = XMVectorScale(XMVectorAdd(xmMin, xmMax), 0.5f);
    auto xmHalfSize = XMVectorScale(XMVectorSubtract(xmMax, xmMin), 0.5f * looseFactor);
    auto xmLooseMin = XMVectorSubtract(xmCenter, xmHalfSize);
    auto xmLooseMax = XMVectorAdd(xmCenter, xmHalfSize);
    XMFLOAT3 looseMin;
    XMFLOAT3 looseMax;
    XMStoreFloat3(&looseMin, xmLooseMin);
    XMStoreFloat3(&looseMax, xmLooseMax);
    bbox = {looseMin, looseMax};
  }

  bool IsValid() const
  {
    return bbox.IsValid() &&
           (IsLeaf() || std::all_of(std::begin(children), std::end(children), [](auto c) {
              return c != nullptr;
            }));
  }

  bool IsLeaf() const
  {
    return std::all_of(std::begin(children), std::end(children), [](auto c) {
      return c == nullptr;
    });
  }

  float BoxSize() const
  {
    using namespace DirectX;
    auto xmMin = XMLoadFloat3(&bbox.min);
    auto xmMax = XMLoadFloat3(&bbox.max);
    auto xmSize = XMVectorSubtract(xmMax, xmMin);
    XMFLOAT3 size;
    XMStoreFloat3(&size, xmSize);
    return std::max({size.x, size.y, size.z});
  }

  float ObjectCount() const { return objects.size(); }

  OctreeNode* GetContainingChild(const AABB& box) const
  {
    for (OctreeNode* child : children) {
      if (child && child->bbox.Contains(box)) {
        return child;
      }
    }
    return nullptr;
  }

  void Attach(ObjectType& obj)
  {
    objects.push_back(&obj);

    obj.octreeNode = this;
    obj.indexInNode = objects.size() - 1;
  }

  void Detach(ObjectType& obj)
  {
    assert(IsValid());
    assert(obj.octreeNode == this);
    assert(!objects.empty());

    size_t index = obj.indexInNode;
    assert(index < objects.size());  // index is valid
    assert(objects[index] == &obj);  // node contains obj in correct position

    // Only swap if it's not the last element (avoids swapping with itself)
    if (index != objects.size() - 1) {
      std::swap(objects[index], objects.back());
    }
    objects.pop_back();

    // Only update if there's still an object at that position
    if (index < objects.size()) {
      objects[index]->indexInNode = index;
    }

    obj.octreeNode = nullptr;
    obj.indexInNode = 0;
  }
};

template<typename ObjectType>
bool ValidateOctree(const OctreeNode<ObjectType>& node)
{
  if (!node.IsValid()) {
    return false;
  }

  for (const ObjectType* obj : node.objects) {
    assert(obj->octreeNode == &node);
    assert(node.bbox.Contains(obj->worldAABB));
  }

  if (!node.IsLeaf()) {
    for (const auto* child : node.children) {
      assert(child);
      assert(ValidateOctree(*child));
    }
  }

  return true;
}

template<typename ObjectType>
void InsertObject(OctreeNode<ObjectType>& node, ObjectType& obj, OctreeNode<ObjectType>& root)
{
  assert(node.IsValid());
  assert(obj.worldAABB.IsValid());
  assert(node.bbox.Contains(obj.worldAABB));

  if (node.IsLeaf()) {
    if (ShouldSubdivide(node.BoxSize(), node.ObjectCount() + 1, node.depth)) {
      const auto& subBoxes = SubdivideBox(node.bbox);
      for (size_t i = 0; i < 8; ++i) {
        assert(!node.children[i]);
        assert(subBoxes[i].IsValid());
        node.children[i] = new OctreeNode<ObjectType>(subBoxes[i], node.depth + 1, 1.5f);
        node.children[i]->parent = &node;
      }

      // Reassign
      auto oldObjects = std::move(node.objects);
      assert(node.objects.empty());

      for (auto* oldObj : oldObjects) {
        InsertObject(root, *oldObj, root);  // Redirect to non-leaf branch
      }

      InsertObject(node, obj, root);  // Redirect to non-leaf branch

    } else {
      // Leaf node that don't need to or cannot be subdivided further accepts the object
      node.Attach(obj);
    }

  } else {
    if (OctreeNode<ObjectType>* child = node.GetContainingChild(obj.worldAABB)) {
      assert(child->IsValid());
      assert(child->bbox.Contains(obj.worldAABB));
      InsertObject(*child, obj, root);
    } else {
      // Cannot push down to children, has to accept in parent
      node.Attach(obj);
    }
  }
}

template<typename ObjectType>
std::unique_ptr<OctreeNode<ObjectType>>
BuildSceneOctreeFromAABB(const AABB& sceneBox, std::vector<ObjectType>& objects)
{
  auto root = std::make_unique<OctreeNode<ObjectType>>(sceneBox, 0);
  for (ObjectType& obj : objects) {
    InsertObject(*root, obj, *root);
  }
  assert(ValidateOctree(*root));
  return root;
}

template<typename ObjectType>
bool UpdateOctreeObject(OctreeNode<ObjectType>& root, ObjectType& obj)
{
  assert(root.IsValid());
  assert(obj.worldAABB.IsValid());
  assert(root.bbox.Contains(obj.worldAABB));
  assert(obj.octreeNode);

  const AABB& objBox = obj.worldAABB;
  OctreeNode<ObjectType>* currentNode = obj.octreeNode;
  assert(currentNode->IsValid());
  assert(currentNode->objects[obj.indexInNode] == &obj);

  if (currentNode->bbox.Contains(objBox)) {
    // Try push down to children if possible
    if (auto* child = currentNode->GetContainingChild(objBox)) {
      currentNode->Detach(obj);
      InsertObject(*child, obj, root);
      return true;
    }
    return false;
  }

  currentNode->Detach(obj);
  InsertObject(root, obj, root);
  return true;
}