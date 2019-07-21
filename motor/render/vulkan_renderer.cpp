#include <cstddef>
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

size_t GetGraphicsQueueIdx(const vk::PhysicalDevice& phy_dev)
{
  std::vector<vk::QueueFamilyProperties> queue_family_props =
      phy_dev.getQueueFamilyProperties();
  CHECK(!queue_family_props.empty()) << "No queues on the device";
  size_t result = -1;
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

vk::Device CreateDevice(const vk::PhysicalDevice& phy_dev,
                        size_t queue_family_idx)
{
  vk::DeviceQueueCreateInfo queue_create_info;
  // Currently we support only a single queue, therefore this is not important.
  const float queue_prios = {.0};
  queue_create_info.setPQueuePriorities(&queue_prios)
      .setQueueCount(1)
      .setQueueFamilyIndex(queue_family_idx)
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

vk::CommandPool CreateCommandPool(const vk::Device& vk_dev,
                                  size_t queue_family_idx)
{
  vk::CommandPoolCreateInfo pool_create_info;
  pool_create_info.setFlags(static_cast<vk::CommandPoolCreateFlags>(0))
      .setQueueFamilyIndex(queue_family_idx)
      .setPNext(nullptr);
  return VkSuccuessOrDie(vk_dev.createCommandPool(pool_create_info),
                         "Couldn't create command pool");
}

std::vector<vk::CommandBuffer> AllocateCommandBuffers(
    const vk::Device& vk_dev, const vk::CommandPool& cmd_pool, size_t count)
{
  vk::CommandBufferAllocateInfo cmd_bufer_alloc_info;
  cmd_bufer_alloc_info.setCommandBufferCount(count)
      .setCommandPool(cmd_pool)
      .setLevel(vk::CommandBufferLevel::ePrimary)
      .setPNext(nullptr);
  return VkSuccuessOrDie(vk_dev.allocateCommandBuffers(cmd_bufer_alloc_info),
                         "Couldn't allocate command buffers");
}

class VulkanRenderer : public Renderer
{
 public:
  VulkanRenderer()
  {
    vk_instance_ = CreateInstance();
    phy_dev_ = SelectPhyiscalDevice(vk_instance_);
    queue_family_idx_ = GetGraphicsQueueIdx(phy_dev_);
    vk_device_ = CreateDevice(phy_dev_, queue_family_idx_);
    vk_cmd_pool_ = CreateCommandPool(vk_device_, queue_family_idx_);

    auto buffers = AllocateCommandBuffers(vk_device_, vk_cmd_pool_, 1);
  }
  void Render() override {}

 private:
  vk::CommandPool vk_cmd_pool_;
  vk::Device vk_device_;
  size_t queue_family_idx_;
  vk::PhysicalDevice phy_dev_;
  vk::Instance vk_instance_;
};

RenderPlugin::Set<VulkanRenderer> x;
}  // namespace
}  // namespace motor
