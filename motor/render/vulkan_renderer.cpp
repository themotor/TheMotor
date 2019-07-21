#include <tuple>

#include "glog/logging.h"
#include "motor/render/renderer.h"
#include "vulkan/vulkan.hpp"

namespace motor
{
namespace
{
vk::Instance CreateInstance()
{
  // TODO(kadircet): Some of the following should come from an options struct.
  vk::ApplicationInfo app_info;
  app_info.setApplicationVersion(1)
      .setApiVersion(VK_API_VERSION_1_1)
      .setEngineVersion(1)
      .setPApplicationName("NABER!")
      .setPEngineName("WUHU")
      .setPNext(nullptr);

  vk::InstanceCreateInfo vk_inst_info;
  vk_inst_info.setPApplicationInfo(&app_info)
      .setEnabledExtensionCount(0)
      .setEnabledLayerCount(0)
      .setPpEnabledExtensionNames(nullptr)
      .setPpEnabledLayerNames(nullptr)
      .setFlags(static_cast<vk::InstanceCreateFlags>(0))
      .setPNext(nullptr);

  vk::Result vk_res;
  vk::Instance vk_inst;
  std::tie(vk_res, vk_inst) = vk::createInstance(vk_inst_info);
  if (vk_res == vk::Result::eErrorIncompatibleDriver)
  {
    LOG(FATAL) << "couldn't find a compatible Vulkan ICD";
  }
  else if (vk_res != vk::Result::eSuccess)
  {
    LOG(FATAL) << "unknown error";
  }
  return vk_inst;
}

class VulkanRenderer : public Renderer
{
 public:
  VulkanRenderer() : vk_instance_(CreateInstance()) {}
  void Render() override {}

 private:
  vk::Instance vk_instance_;
};

RenderPlugin::Set<VulkanRenderer> x;
}  // namespace
}  // namespace motor
