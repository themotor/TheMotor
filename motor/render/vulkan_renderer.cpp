#include <cstddef>
#include <tuple>
#include <vector>

#include "glog/logging.h"
#include "motor/render/renderer.h"
#include "vulkan/vulkan.hpp"
// NOLINT
#include "GLFW/glfw3.h"

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

  const std::vector<const char*> extensions = {
      VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
      VK_KHR_XCB_SURFACE_EXTENSION_NAME,
#endif
  };
  vk::InstanceCreateInfo vk_inst_info;
  vk_inst_info.setPApplicationInfo(&app_info)
      .setEnabledLayerCount(0)
      .setPpEnabledLayerNames(nullptr)
      .setEnabledExtensionCount(extensions.size())
      .setPpEnabledExtensionNames(extensions.data())
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

size_t GetPresentQueueIdx(const vk::PhysicalDevice& phy_dev,
                          const vk::SurfaceKHR& surface)
{
  std::vector<vk::QueueFamilyProperties> queue_family_props =
      phy_dev.getQueueFamilyProperties();
  CHECK(!queue_family_props.empty()) << "No queues on the device";
  size_t result = -1;
  for (size_t i = 0; i < queue_family_props.size(); ++i)
  {
    if (VkSuccuessOrDie(phy_dev.getSurfaceSupportKHR(i, surface),
                        "Couldn't query for KHR support"))
      result = i;
  }
  CHECK(result != -1) << "No queue with present capability";
  return result;
}

vk::SurfaceKHR CreateSurface(const vk::Instance& inst)
{
  VkSurfaceKHR vk_surface;
  CHECK(glfwCreateWindowSurface(
            inst,
            static_cast<GLFWwindow*>(
                glfwGetMonitorUserPointer(glfwGetPrimaryMonitor())),
            nullptr, &vk_surface) == VK_SUCCESS)
      << "Failed to create surface";
  CHECK(vk_surface != VK_NULL_HANDLE) << "Null surface";
  return vk_surface;
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

  const std::vector<const char*> extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  vk::DeviceCreateInfo create_info;
  create_info.setPQueueCreateInfos(&queue_create_info)
      .setQueueCreateInfoCount(1)
      .setEnabledExtensionCount(extensions.size())
      .setPpEnabledExtensionNames(extensions.data())
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

vk::SwapchainKHR CreateSwapChain(const vk::PhysicalDevice& phy_dev,
                                 const vk::SurfaceKHR& vk_surface,
                                 const vk::Device& vk_device)
{
  std::vector<vk::SurfaceFormatKHR> formats = VkSuccuessOrDie(
      phy_dev.getSurfaceFormatsKHR(vk_surface), "Couldn't get surface formats");
  CHECK(!formats.empty()) << "No surface formats";
  // Surface has no preferences, pick a common format.
  if (formats[0].format == vk::Format::eUndefined)
  {
    formats[0].format = vk::Format::eB8G8R8A8Snorm;
  }

  vk::SurfaceCapabilitiesKHR surf_caps =
      VkSuccuessOrDie(phy_dev.getSurfaceCapabilitiesKHR(vk_surface),
                      "Couldn't get surface capabilities");
  std::vector<vk::PresentModeKHR> present_modes =
      VkSuccuessOrDie(phy_dev.getSurfacePresentModesKHR(vk_surface),
                      "Couldn't get present modes");

  vk::Extent2D swap_chain_extend;
  // TODO(kadircet): Use values coming from user instead.
  if (surf_caps.currentExtent.height == 0xFFFFFFFF)
  {
    LOG(INFO) << "Current extent unknown setting to 800x600";
    surf_caps.currentExtent.setHeight(600);
    surf_caps.currentExtent.setWidth(800);
  }
  swap_chain_extend = surf_caps.currentExtent;

  vk::SwapchainCreateInfoKHR swapchain_info;
  swapchain_info.setSurface(vk_surface)
      .setImageFormat(formats[0].format)
      .setMinImageCount(surf_caps.minImageCount)
      .setImageExtent(swap_chain_extend)
      // FIFO is supported by all devices.
      .setPresentMode(vk::PresentModeKHR::eFifo)
      .setPreTransform(surf_caps.currentTransform)
      .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eInherit)
      .setImageArrayLayers(1)
      .setOldSwapchain(nullptr)
      .setClipped(true)
      .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
      .setImageSharingMode(vk::SharingMode::eExclusive)
      .setQueueFamilyIndexCount(0)
      .setPQueueFamilyIndices(nullptr)
      .setPNext(nullptr);

  return VkSuccuessOrDie(vk_device.createSwapchainKHR(swapchain_info),
                         "Couldn't create swapchain");
}

class VulkanRenderer : public Renderer
{
 public:
  VulkanRenderer()
  {
    vk_instance_ = CreateInstance();
    vk_surface_ = CreateSurface(vk_instance_);
    phy_dev_ = SelectPhyiscalDevice(vk_instance_);
    queue_graphics_family_idx_ = GetGraphicsQueueIdx(phy_dev_);
    queue_present_family_idx_ = GetPresentQueueIdx(phy_dev_, vk_surface_);
    CHECK(queue_graphics_family_idx_ == queue_present_family_idx_)
        << "Different family indices for present and graphics are not "
           "supported yet.";

    vk_device_ = CreateDevice(phy_dev_, queue_graphics_family_idx_);
    vk_cmd_pool_ = CreateCommandPool(vk_device_, queue_graphics_family_idx_);

    auto buffers = AllocateCommandBuffers(vk_device_, vk_cmd_pool_, 1);
    vk_swapchain_ = CreateSwapChain(phy_dev_, vk_surface_, vk_device_);

    vk_images_ = VkSuccuessOrDie(
        vk_device_.getSwapchainImagesKHR(vk_swapchain_), "Couldn't get images");
  }
  void Render() override {}

  ~VulkanRenderer() final { vk_instance_.destroy(); }

 private:
  std::vector<vk::Image> vk_images_;
  vk::SwapchainKHR vk_swapchain_;
  vk::CommandPool vk_cmd_pool_;
  vk::Device vk_device_;
  size_t queue_present_family_idx_;
  size_t queue_graphics_family_idx_;
  vk::PhysicalDevice phy_dev_;
  vk::SurfaceKHR vk_surface_;
  vk::Instance vk_instance_;
};

RenderPlugin::Set<VulkanRenderer> x;
}  // namespace
}  // namespace motor
