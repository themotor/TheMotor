#include <bits/stdint-uintn.h>

#include <cstddef>
#include <limits>
#include <set>
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
void VkSuccuessOrDie(vk::Result res, const char* desc)
{
  CHECK(res == vk::Result::eSuccess)
      << "VK error: " << desc << '-' << static_cast<int>(res);
}

struct DepthBuffer
{
  vk::Format format_;
  vk::Image image_;
  vk::DeviceMemory memory_;
  vk::ImageView view_;
};

struct QueueFamilyIndices
{
  std::optional<size_t> graphics_queue_idx;
  std::optional<size_t> present_queue_idx;

  bool isComplete()
  {
    return graphics_queue_idx.has_value() && present_queue_idx.has_value();
  }
};

std::vector<const char*> GetRequiredExtensions()
{
  std::vector<const char*> extensions;
  uint32_t glfw_extension_count = 0;
  const char** glfw_extensions =
      glfwGetRequiredInstanceExtensions(&glfw_extension_count);
  CHECK(glfw_extensions) << "Vulkan is not supported on the machine";
  extensions.resize(glfw_extension_count);
  std::copy(glfw_extensions, glfw_extensions + glfw_extension_count,
            extensions.begin());

  return extensions;
}

QueueFamilyIndices GetQueueIndices(const vk::PhysicalDevice& phy_dev,
                                   const vk::SurfaceKHR& surface)
{
  QueueFamilyIndices indices;
  std::vector<vk::QueueFamilyProperties> queue_family_props =
      phy_dev.getQueueFamilyProperties();
  CHECK(!queue_family_props.empty()) << "No queues on the device";

  for (size_t i = 0; i < queue_family_props.size(); ++i)
  {
    const auto& family_prop = queue_family_props[i];
    if (family_prop.queueCount &&
        family_prop.queueFlags & vk::QueueFlagBits::eGraphics)
    {
      indices.graphics_queue_idx = i;
    }
    if (VkSuccuessOrDie(phy_dev.getSurfaceSupportKHR(i, surface),
                        "Couldn't query for KHR support"))
    {
      indices.present_queue_idx = i;
    }
  }
  return indices;
}

bool CheckExtensionSupport(const vk::PhysicalDevice& phy_dev)
{
  std::vector<vk::ExtensionProperties> available_extensions =
      VkSuccuessOrDie(phy_dev.enumerateDeviceExtensionProperties(),
                      "Coudn't get device extensions");
  // device_extensions is also used when creating logical device, maybe move it
  // to global? Or maybe make these functions part of the VulkanRenderer class?
  const std::vector<const char*> device_extensions = {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  std::set<std::string> required_ext(device_extensions.begin(),
                                     device_extensions.end());
  for (const auto& extension : available_extensions)
  {
    required_ext.erase(extension.extensionName);
  }
  return required_ext.empty();
}

bool CheckSwapchainSupport(const vk::PhysicalDevice& phy_dev,
                           const vk::SurfaceKHR& surface)
{
  std::vector<vk::SurfaceFormatKHR> formats = VkSuccuessOrDie(
      phy_dev.getSurfaceFormatsKHR(surface), "Couldn't get surface formats");
  std::vector<vk::PresentModeKHR> present_modes = VkSuccuessOrDie(
      phy_dev.getSurfacePresentModesKHR(surface), "Couldn't get present modes");
  return !formats.empty() && !present_modes.empty();
}

bool isSuitableDevice(const vk::PhysicalDevice& phy_dev,
                      const vk::SurfaceKHR& surface)
{
  QueueFamilyIndices queue_indices = GetQueueIndices(phy_dev, surface);
  if (!queue_indices.isComplete()) return false;
  bool isExtensionsSupported = CheckExtensionSupport(phy_dev);
  if (!isExtensionsSupported) return false;
  bool isSwapchainOk = CheckSwapchainSupport(phy_dev, surface);
  if (!isSwapchainOk) return false;
  return true;
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

  const std::vector<const char*> extensions = GetRequiredExtensions();
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

vk::PhysicalDevice SelectPhyiscalDevice(const vk::Instance& vk_inst,
                                        const vk::SurfaceKHR& surface)
{
  std::vector<vk::PhysicalDevice> devices = VkSuccuessOrDie(
      vk_inst.enumeratePhysicalDevices(), "Couldn't enumeratePhysicalDevices");
  CHECK(!devices.empty()) << "No vulkan device found";
  vk::PhysicalDevice suitable_device;
  for (const vk::PhysicalDevice& phy_dev : devices)
  {
    const vk::PhysicalDeviceProperties props = phy_dev.getProperties();
    LOG(INFO) << "Found device: " << props.deviceName
              << " api: " << props.apiVersion
              << " driver: " << props.driverVersion;
    if (isSuitableDevice(phy_dev, surface))
    {
      suitable_device = phy_dev;
      // TODO(hbostann): If we don't want to list all devices, maybe break the
      // loop on first suitable device?
    }
  }
  return suitable_device;
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
                                 const vk::Device& vk_device,
                                 vk::Format* format)
{
  std::vector<vk::SurfaceFormatKHR> formats = VkSuccuessOrDie(
      phy_dev.getSurfaceFormatsKHR(vk_surface), "Couldn't get surface formats");
  CHECK(!formats.empty()) << "No surface formats";
  // Surface has no preferences, pick a common format.
  // TODO(hbostann): Implement a better selection preferring surface formats
  // with vk::Format::eB8G8R8A8Snorm and vk::ColorSpaceKHR::eSrgbNonlinear
  if (formats[0].format == vk::Format::eUndefined)
  {
    formats[0].format = vk::Format::eB8G8R8A8Snorm;
  }
  *format = formats[0].format;

  vk::SurfaceCapabilitiesKHR surf_caps =
      VkSuccuessOrDie(phy_dev.getSurfaceCapabilitiesKHR(vk_surface),
                      "Couldn't get surface capabilities");
  std::vector<vk::PresentModeKHR> present_modes =
      VkSuccuessOrDie(phy_dev.getSurfacePresentModesKHR(vk_surface),
                      "Couldn't get present modes");

  vk::Extent2D swap_chain_extent;
  // TODO(kadircet): Use values coming from user instead.
  if (surf_caps.currentExtent.height == UINT32_MAX)
  {
    LOG(INFO) << "Current extent unknown setting to 800x600";
    uint32_t width = 800;
    uint32_t height = 600;
    surf_caps.currentExtent.setWidth(
        std::max(surf_caps.minImageExtent.width,
                 std::min(surf_caps.maxImageExtent.width, width)));
    surf_caps.currentExtent.setHeight(
        std::max(surf_caps.minImageExtent.height,
                 std::min(surf_caps.maxImageExtent.height, height)));
  }
  swap_chain_extent = surf_caps.currentExtent;

  uint32_t image_count = surf_caps.minImageCount + 1;
  if (surf_caps.maxImageCount > 0 && image_count > surf_caps.maxImageCount)
  {
    image_count = surf_caps.maxImageCount;
  }

  // FIFO is supported by all devices.
  vk::PresentModeKHR present_mode = vk::PresentModeKHR::eFifo;
  for (const auto& available_present_mode : present_modes)
  {
    if (available_present_mode == vk::PresentModeKHR::eMailbox)
    {
      present_mode = available_present_mode;
    }
  }

  vk::SwapchainCreateInfoKHR swapchain_info;
  swapchain_info.setSurface(vk_surface)
      .setMinImageCount(image_count)
      .setImageFormat(formats[0].format)
      .setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
      .setImageExtent(swap_chain_extent)
      .setImageArrayLayers(1)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
      .setImageSharingMode(vk::SharingMode::eExclusive)
      // TODO(hbostann): Handle cases where graphics family and present family
      // indexes are different
      //                  - Make ImageSharingMode = vk::SharingMode::eConcurrent
      //                  - Fill in QueueFamilyIndexCount and
      //                  PQueueFamilyIndices
      .setQueueFamilyIndexCount(0)
      .setPQueueFamilyIndices(nullptr)
      .setPreTransform(surf_caps.currentTransform)
      .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eInherit)
      .setPresentMode(present_mode)
      .setClipped(true)
      .setOldSwapchain(nullptr)
      .setPNext(nullptr);

  return VkSuccuessOrDie(vk_device.createSwapchainKHR(swapchain_info),
                         "Couldn't create swapchain");
}

std::vector<vk::ImageView> CreateImageViews(
    const vk::Device& vk_dev, const std::vector<vk::Image>& vk_images,
    const vk::Format& vk_format)
{
  std::vector<vk::ImageView> image_views;
  for (const vk::Image& image : vk_images)
  {
    vk::ImageViewCreateInfo image_view_info;
    image_view_info.setImage(image)
        .setViewType(vk::ImageViewType::e2D)
        .setComponents(vk::ComponentMapping(
            vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
            vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
        .setFlags(static_cast<vk::ImageViewCreateFlags>(0))
        .setFormat(vk_format)
        .setSubresourceRange(vk::ImageSubresourceRange(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
    image_views.push_back(VkSuccuessOrDie(
        vk_dev.createImageView(image_view_info), "Couldn't create imageview"));
  }
  return image_views;
}

vk::RenderPass CreateRenderPass(const vk::Device& vk_dev,
                                const vk::Format& vk_format)
{
  vk::AttachmentDescription color_attachment;
  color_attachment.setFormat(vk_format)
      .setSamples(vk::SampleCountFlagBits::e1)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
      .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentReference color_attachment_ref;
  color_attachment_ref.setAttachment(0).setLayout(
      vk::ImageLayout::eColorAttachmentOptimal);

  vk::SubpassDescription subpass;
  subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
      .setColorAttachmentCount(1)
      .setPColorAttachments(&color_attachment_ref);

  vk::RenderPassCreateInfo render_pass_info;
  render_pass_info.setAttachmentCount(1)
      .setPAttachments(&color_attachment)
      .setSubpassCount(1)
      .setPSubpasses(&subpass);

  return VkSuccuessOrDie(vk_dev.createRenderPass(render_pass_info),
                         "Couldn't create Render Pass!");
}

DepthBuffer CreateDepthBuffer(const vk::PhysicalDevice& phy_dev,
                              const vk::Device& dev)
{
  DepthBuffer depth_buffer;
  depth_buffer.format_ = vk::Format::eD16Unorm;

  vk::FormatProperties format_props =
      phy_dev.getFormatProperties(depth_buffer.format_);
  vk::ImageTiling tiling;
  if (format_props.linearTilingFeatures &
      vk::FormatFeatureFlagBits::eDepthStencilAttachment)
  {
    tiling = vk::ImageTiling::eLinear;
  }
  else if (format_props.optimalTilingFeatures &
           vk::FormatFeatureFlagBits::eDepthStencilAttachment)
  {
    tiling = vk::ImageTiling::eOptimal;
  }
  else
  {
    LOG(FATAL) << "16bit norm format is not supported";
  }

  vk::ImageCreateInfo image_create_info;
  image_create_info.setImageType(vk::ImageType::e2D)
      .setArrayLayers(1)
      .setExtent(vk::Extent3D(800, 600, 1))
      .setFlags(static_cast<vk::ImageCreateFlags>(0))
      .setFormat(depth_buffer.format_)
      .setInitialLayout(vk::ImageLayout::eUndefined)
      .setMipLevels(1)
      .setPQueueFamilyIndices(nullptr)
      .setQueueFamilyIndexCount(0)
      .setSamples(vk::SampleCountFlagBits::e1)
      .setSharingMode(vk::SharingMode::eExclusive)
      .setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment)
      .setTiling(tiling)
      .setPNext(nullptr);
  depth_buffer.image_ = VkSuccuessOrDie(dev.createImage(image_create_info),
                                        "Couldn't create depth buffer");

  vk::MemoryRequirements mem_reqs =
      dev.getImageMemoryRequirements(depth_buffer.image_);
  size_t type_bits = mem_reqs.memoryTypeBits;

  int memory_type_id = -1;
  vk::PhysicalDeviceMemoryProperties mem_props = phy_dev.getMemoryProperties();
  for (int i = 0; i < mem_props.memoryTypeCount; ++i, type_bits >>= 1)
  {
    if ((type_bits & 1) == 0) continue;
    if (mem_props.memoryTypes[i].propertyFlags &
        vk::MemoryPropertyFlagBits::eDeviceLocal)
    {
      memory_type_id = i;
      break;
    }
  }
  CHECK(memory_type_id != -1) << "Couldn't find device local memory";

  vk::MemoryAllocateInfo memory_alloc_info;
  memory_alloc_info.setAllocationSize(mem_reqs.size)
      .setMemoryTypeIndex(memory_type_id)
      .setPNext(nullptr);
  depth_buffer.memory_ = VkSuccuessOrDie(dev.allocateMemory(memory_alloc_info),
                                         "Couldn't allocate memory");

  VkSuccuessOrDie(
      dev.bindImageMemory(depth_buffer.image_, depth_buffer.memory_, 0),
      "Couldn't bind memory");

  vk::ImageViewCreateInfo view_create_info;
  view_create_info.setImage(depth_buffer.image_)
      .setFormat(depth_buffer.format_)
      .setComponents(vk::ComponentMapping(
          vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
          vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA))
      .setSubresourceRange(vk::ImageSubresourceRange(
          vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
      .setViewType(vk::ImageViewType::e2D)
      .setFlags(static_cast<vk::ImageViewCreateFlags>(0))
      .setPNext(nullptr);
  depth_buffer.view_ = VkSuccuessOrDie(dev.createImageView(view_create_info),
                                       "Couldn't create image view");

  return depth_buffer;
}

class VulkanRenderer : public Renderer
{
 public:
  VulkanRenderer()
  {
    vk_instance_ = CreateInstance();
    vk_surface_ = CreateSurface(vk_instance_);
    phy_dev_ = SelectPhyiscalDevice(vk_instance_, vk_surface_);
    queue_indices_ = GetQueueIndices(phy_dev_, vk_surface_);
    CHECK(queue_indices_.isComplete())
        << "Can't find one or more queue indices";
    CHECK(queue_indices_.graphics_queue_idx.value() ==
          queue_indices_.present_queue_idx.value())
        << "Different family indices for present and graphics are not "
           "supported yet.";
    vk_device_ =
        CreateDevice(phy_dev_, queue_indices_.graphics_queue_idx.value());
    vk_swapchain_ =
        CreateSwapChain(phy_dev_, vk_surface_, vk_device_, &vk_format_);
    vk_images_ = VkSuccuessOrDie(
        vk_device_.getSwapchainImagesKHR(vk_swapchain_), "Couldn't get images");

    vk_image_views_ = CreateImageViews(vk_device_, vk_images_, vk_format_);
    vk_render_pass_ = CreateRenderPass(vk_device_, vk_format_);

    vk_cmd_pool_ = CreateCommandPool(vk_device_,
                                     queue_indices_.graphics_queue_idx.value());

    depth_buffer_ = CreateDepthBuffer(phy_dev_, vk_device_);

    cmd_buffers_ =
        AllocateCommandBuffers(vk_device_, vk_cmd_pool_, vk_images_.size());

    vk::ClearColorValue color;
    color.setFloat32({.0, .0, 1., .0});

    vk::ImageSubresourceRange image_range;
    image_range.setAspectMask(vk::ImageAspectFlagBits::eColor)
        .setLevelCount(1)
        .setLayerCount(1);

    vk::CommandBufferBeginInfo cmd_buf_begin_info;
    cmd_buf_begin_info.setFlags(
        vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    for (size_t i = 0; i < cmd_buffers_.size(); ++i)
    {
      auto& cmd_buffer = cmd_buffers_[i];
      VkSuccuessOrDie(cmd_buffer.begin(cmd_buf_begin_info),
                      "Couldn't start command buffer");
      cmd_buffer.clearColorImage(vk_images_[i], vk::ImageLayout::eGeneral,
                                 &color, 1, &image_range);

      VkSuccuessOrDie(cmd_buffer.end(), "Couldn't end command buffer");
    }

    vk_queue_ =
        vk_device_.getQueue(queue_indices_.graphics_queue_idx.value(), 0);
  }

  void Render() override
  {
    uint32_t next_image_idx =
        VkSuccuessOrDie(vk_device_.acquireNextImageKHR(
                            vk_swapchain_, std::numeric_limits<uint64_t>::max(),
                            nullptr, nullptr),
                        "Couldn't acquire next image");
    vk::SubmitInfo submit_info;
    submit_info.setPNext(nullptr).setCommandBufferCount(1).setPCommandBuffers(
        &cmd_buffers_[next_image_idx]);
    VkSuccuessOrDie(vk_queue_.submit({submit_info}, nullptr),
                    "Couldn't submit to the queue");

    vk::PresentInfoKHR present_info;
    present_info.setSwapchainCount(1)
        .setPSwapchains(&vk_swapchain_)
        .setPImageIndices(&next_image_idx);
    VkSuccuessOrDie(vk_queue_.presentKHR(present_info), "Couldn't present");
  }

  ~VulkanRenderer() final
  {
    vk_device_.freeCommandBuffers(vk_cmd_pool_, cmd_buffers_);

    vk_device_.destroyImageView(depth_buffer_.view_);
    vk_device_.destroyImage(depth_buffer_.image_);
    vk_device_.freeMemory(depth_buffer_.memory_);

    for (vk::ImageView& img_view : vk_image_views_)
    {
      vk_device_.destroyImageView(img_view);
    }
    // This also destroys all of the vk_image_ handles
    vk_device_.destroySwapchainKHR(vk_swapchain_);

    vk_device_.destroyCommandPool(vk_cmd_pool_);

    vk_device_.waitIdle();
    vk_device_.destroy();

    vk_instance_.destroy(vk_surface_);
    vk_instance_.destroy();
  }

 private:
  vk::Queue vk_queue_;
  std::vector<vk::CommandBuffer> cmd_buffers_;
  DepthBuffer depth_buffer_;
  std::vector<vk::ImageView> vk_image_views_;
  std::vector<vk::Image> vk_images_;
  vk::Format vk_format_;
  vk::SwapchainKHR vk_swapchain_;
  vk::RenderPass vk_render_pass_;
  vk::CommandPool vk_cmd_pool_;
  vk::Device vk_device_;
  QueueFamilyIndices queue_indices_;
  vk::PhysicalDevice phy_dev_;
  vk::SurfaceKHR vk_surface_;
  vk::Instance vk_instance_;
};

RenderPlugin::Set<VulkanRenderer> x;
}  // namespace
}  // namespace motor
