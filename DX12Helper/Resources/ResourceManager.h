#pragma once

#include <unordered_map>

#include "Buffers.h"
#include "DescriptorHeap.h"
#include "Device.h"
#include "Resources.h"
#include "Textures.h"


namespace dxh
{


class BufferManager
{
public:
  explicit BufferManager(Device& device) : device{device}, cbvSrvUavPool{device.Get()} {}

  void Register(const std::shared_ptr<Buffer>& resource)
  {
    entries[resource.get()].resource = resource;
    device.CreateCBV(*resource, cbvSrvUavPool.Allocate());
  }

  std::shared_ptr<Buffer> Find(const Buffer* resource)
  {
    if (entries.count(resource)) {
      return entries[resource].resource;
    }
    return nullptr;
  }

  std::shared_ptr<Buffer> FindByName(const std::string& name)
  {
    for (const auto& pair : entries) {
      if (pair.second.resource->Name() == name) {
        return pair.second.resource;
      }
    }
    return nullptr;
  }

  D3D12_CPU_DESCRIPTOR_HANDLE GetCBV(const Buffer* resource)
  {
    if (entries.count(resource)) {
      return entries[resource].cbv;
    }
    return {};
  }

private:
  struct Entry {
    std::shared_ptr<Buffer> resource;
    D3D12_CPU_DESCRIPTOR_HANDLE cbv;
  };

  Device& device;
  std::unordered_map<const Buffer*, Entry> entries;
  dxh::CbvSrvUavPool cbvSrvUavPool;
};

}  // namespace dxh