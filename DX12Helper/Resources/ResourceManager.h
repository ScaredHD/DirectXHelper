#pragma once

#include <unordered_map>

#include "Resources.h"


class ResourceManager
{
public:
  enum class CollisionResponse : uint8_t { Ignore, Rename, Error };

  bool Register(
    const std::shared_ptr<dxh::TrackedResource>& resource,
    CollisionResponse collisionResponse = CollisionResponse::Ignore
  )
  {
    if (!resource) {
      return false;
    }

    if (Find(resource->Name())) {
      if (collisionResponse == CollisionResponse::Rename) {
        size_t suffix = 1;
        auto rename = [&suffix](const std::string& name) {
          return name + "_" + std::to_string(suffix);
        };

        while (Find(rename(resource->Name()))) {
          if (suffix > 1000) {
            return false;
          }
          ++suffix;
        }
        resource->Rename(rename(resource->Name()));
      } else if (collisionResponse == CollisionResponse::Error) {
        throw std::runtime_error("Resource with name '" + resource->Name() + "' already exists.");
      } else if (collisionResponse == CollisionResponse::Ignore) {
        return false;
      }
    }

    nameToRes[resource->Name()] = resource;
    return true;
  }

  std::shared_ptr<dxh::TrackedResource> Find(const std::string& name) const
  {
    auto it = nameToRes.find(name);
    if (it != nameToRes.end()) {
      return it->second;
    }
    return nullptr;
  }

  size_t Count() const { return nameToRes.size(); }

  std::vector<std::string> ManagedResourceNames() const
  {
    std::vector<std::string> names;
    names.reserve(nameToRes.size());
    for (const auto& pair : nameToRes) {
      names.push_back(pair.first);
    }
    return names;
  }

  void CleanupUnusedResources()
  {
    for (auto it = nameToRes.begin(); it != nameToRes.end();) {
      if (it->second.use_count() == 1) {
        it = nameToRes.erase(it);
      } else {
        ++it;
      }
    }
  }


private:
  std::unordered_map<std::string, std::shared_ptr<dxh::TrackedResource>> nameToRes;
};