#include <tuple>
#include <vector>

#include "glog/logging.h"
#include "motor/render/renderer.h"
#include "vulkan/vulkan.hpp"

namespace motor
{
namespace
{
template <typename T>
T VkSuccuessOrDie(vk::ResultValue<T> res_and_val, const char* desc)
{
  CHECK(res_and_val.result == vk::Result::eSuccess)
      << "VK error: " << desc << '-' << static_cast<int>(res_and_val.result);
  return res_and_val.value;
}

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

  return VkSuccuessOrDie(vk::createInstance(vk_inst_info),
                         "Couldn't createInstance");
}

vk::PhysicalDevice SelectPhyiscalDevice(const vk::Instance& vk_inst)
{
  std::vector<vk::PhysicalDevice> devices = VkSuccuessOrDie(
      vk_inst.enumeratePhysicalDevices(), "Couldn't enumeratePhysicalDevices");
  CHECK(!devices.empty()) << "No vulkan device found";

  for (const vk::PhysicalDevice& phy_dev : devices)
  {
    const vk::PhysicalDeviceProperties props = phy_dev.getProperties();
    LOG(INFO) << "Found device: " << props.deviceName
              << " api: " << props.apiVersion
              << " driver: " << props.driverVersion;
  }
  // TODO(kadircet): I suppose we'll need a more clever selection logic.
  return devices.front();
}

int GetGraphicsQueueIdx(const vk::PhysicalDevice& phy_dev)
{
  std::vector<vk::QueueFamilyProperties> queue_family_props =
      phy_dev.getQueueFamilyProperties();
  CHECK(!queue_family_props.empty()) << "No queues on the device";
  int result = -1;
  for (int i = 0; i < queue_family_props.size(); ++i)
  {
    const auto& family_prop = queue_family_props[i];
    LOG(INFO) << "Queue: " << family_prop.queueCount << ' '
              << static_cast<unsigned>(family_prop.queueFlags);
    if (family_prop.queueFlags & vk::QueueFlagBits::eGraphics) result = i;
  }
  CHECK(result != -1) << "No queue with a graphics flag";
  return result;
}

vk::Device CreateDevice(const vk::Instance& vk_inst)
{
  vk::PhysicalDevice phy_dev = SelectPhyiscalDevice(vk_inst);

  vk::DeviceQueueCreateInfo queue_create_info;
  // Currently we support only a single queue, therefore this is not important.
  const float queue_prios = {.0};
  queue_create_info.setPQueuePriorities(&queue_prios)
      .setQueueCount(1)
      .setQueueFamilyIndex(GetGraphicsQueueIdx(phy_dev))
      .setFlags(static_cast<vk::DeviceQueueCreateFlags>(0))
      .setPNext(nullptr);

  vk::DeviceCreateInfo create_info;
  create_info.setPQueueCreateInfos(&queue_create_info)
      .setQueueCreateInfoCount(1)
      .setEnabledExtensionCount(0)
      .setPpEnabledExtensionNames(nullptr)
      // Layers are deprecated.
      .setEnabledLayerCount(0)
      .setPpEnabledLayerNames(nullptr)
      .setPEnabledFeatures(nullptr)
      .setPNext(nullptr);

  return VkSuccuessOrDie(phy_dev.createDevice(create_info),
                         "Couldn't create logical device");
}

class VulkanRenderer : public Renderer
{
 public:
  VulkanRenderer()
  {
    vk_instance_ = CreateInstance();
    vk_device_ = CreateDevice(vk_instance_);
  }
  void Render() override {}

 private:
  vk::Device vk_device_;
  vk::Instance vk_instance_;
};

RenderPlugin::Set<VulkanRenderer> x;
}  // namespace
}  // namespace motor
