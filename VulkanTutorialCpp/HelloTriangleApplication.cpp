#include "HelloTriangleApplication.h"

#include <functional>
#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <chrono>
#include <unordered_map>

#define STB_IMAGE_IMPLEMENTATION
#include <stb\stb_image.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobj\tiny_obj_loader.h>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::string MODEL_PATH = "models/chalet.obj";
const std::string TEXTURE_PATH = "textures/chalet.jpg";

const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
const bool enableValidationLayers = false
#else
const bool enableValidationLayers = true;
#endif

static void getRequiredGlfwExtensions (std::vector<const char *> &extensions)
{
   unsigned int glfwExtensionCount = 0;
   const char** glfwExtensions;

   glfwExtensions = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);

   extensions.insert (extensions.end (), glfwExtensions, glfwExtensions + glfwExtensionCount);
}

static std::vector<char> readFile (const std::string& filename)
{
   std::ifstream file (filename, std::ios::ate | std::ios::binary);

   if (!file.is_open ())
   {
      throw std::runtime_error ("failed to open file!");
   }

   size_t fileSize = (size_t) file.tellg ();
   std::vector<char> buffer (fileSize);

   file.seekg (0);
   file.read (buffer.data (), fileSize);

   file.close ();

   return buffer;
}


HelloTriangleApplication::HelloTriangleApplication ()
{
}


HelloTriangleApplication::~HelloTriangleApplication ()
{
}

void HelloTriangleApplication::run ()
{
   initWindow ();
   initVulkan ();
   mainLoop ();
   cleanup ();
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback (
   VkDebugReportFlagsEXT flags,
   VkDebugReportObjectTypeEXT objType,
   uint64_t obj,
   size_t location,
   int32_t code,
   const char * layerPrefix,
   const char * msg,
   void * userData)
{
   std::cerr << "validation layer: " << msg << std::endl;

   return VK_FALSE;
}

void HelloTriangleApplication::initWindow ()
{
   glfwInit ();

   glfwWindowHint (GLFW_CLIENT_API, GLFW_NO_API);
   glfwWindowHint (GLFW_RESIZABLE, GLFW_TRUE);

   window = glfwCreateWindow (WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

   glfwSetWindowUserPointer (window, this);
   glfwSetWindowSizeCallback (window, HelloTriangleApplication::onWindowResized);
}

void HelloTriangleApplication::onWindowResized (GLFWwindow * window, int width, int height)
{
   if (width == 0 || height == 0) return;

   auto app = reinterpret_cast<HelloTriangleApplication*> (glfwGetWindowUserPointer (window));

   app->recreateSwapChain ();

}


void HelloTriangleApplication::initVulkan ()
{
   createInstance ();
   setupDebugCallback ();
   createSurface ();
   pickPhysicalDevice ();
   createLogicalDevice ();
   createSwapChain ();
   createImageViews ();
   createRenderPass ();
   createDescriptorSetLayout ();
   createGraphicsPipeline ();
   createCommandPool ();
   createDepthResources ();
   createFramebuffers ();
   createTextureImage ();
   createTextureImageView ();
   createTextureSampler ();
   loadModel ();
   createVertexBuffer ();
   createIndexBuffer ();
   createUniformBuffer ();
   createDescriptorPool ();
   createDescriptorSet ();
   createCommandBuffers ();
   createSemaphores ();
}

void HelloTriangleApplication::createInstance ()
{
   if (enableValidationLayers)
   {
      PrintSupportedValidationLayers ();
      checkValidationLayerSupport ();
   }


   vk::ApplicationInfo appInfo;

   appInfo.setPApplicationName ("Hello Triangle");
   appInfo.setApplicationVersion (VK_MAKE_VERSION (1, 0, 0));
   appInfo.setPEngineName ("No Engine");
   appInfo.setEngineVersion (VK_MAKE_VERSION (1, 0, 0));
   appInfo.setApiVersion (VK_API_VERSION_1_0);

   vk::InstanceCreateInfo createInfo = {};

   createInfo.setPApplicationInfo (&appInfo);

   PrintSupportedInstanceExtensions ();

   auto extensions = getRequiredExtensions ();
   checkInstanceExtensionSupport (extensions);

   createInfo.setPpEnabledExtensionNames (extensions.data ());
   createInfo.setEnabledExtensionCount (static_cast<uint32_t> (extensions.size ()));

   if (enableValidationLayers)
   {
      createInfo.setEnabledLayerCount (static_cast<uint32_t> (validationLayers.size ()));
      createInfo.setPpEnabledLayerNames (validationLayers.data ());
   }
   else
   {
      createInfo.setEnabledLayerCount (0);
   }

   try
   {
      instance = vk::createInstance (createInfo, nullptr);
   }
   catch (std::exception const&)
   {
      throw std::runtime_error ("failed to create instance!");
   }


}

void HelloTriangleApplication::setupDebugCallback ()
{
   if (!enableValidationLayers) return;

   auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) instance.getProcAddr ("vkCreateDebugReportCallbackEXT");
   auto vkDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT) instance.getProcAddr ("vkDestroyDebugReportCallbackEXT");

   if (vkCreateDebugReportCallbackEXT)
   {
      vk::DebugReportCallbackCreateInfoEXT createInfo (
         vk::DebugReportFlagsEXT (
            vk::DebugReportFlagBitsEXT::eWarning |
            vk::DebugReportFlagBitsEXT::eError
         ),
         debugCallback,
         nullptr
      );


      //auto pinfo = reinterpret_cast<const VkDebugReportCallbackCreateInfoEXT*> (&createInfo);
      if (CreateDebugReportCallbackEXT (
         VkInstance (instance),
         reinterpret_cast<VkDebugReportCallbackCreateInfoEXT*>(&createInfo),
         nullptr,
         reinterpret_cast<VkDebugReportCallbackEXT*>(&callback)) != VK_SUCCESS)
      {
         throw std::runtime_error ("failed to set up debug callback!");
      }
   }

}

void HelloTriangleApplication::createSurface ()
{
   if (glfwCreateWindowSurface (VkInstance (instance), window, nullptr, reinterpret_cast<VkSurfaceKHR*> (&surface)) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create window surface!");
   }
}

void HelloTriangleApplication::pickPhysicalDevice ()
{
   PrintAvailablePhysicalDevices (instance);

   std::vector<vk::PhysicalDevice> devices = instance.enumeratePhysicalDevices ();

   if (devices.empty ())
   {
      throw std::runtime_error ("failed to find GPUs with Vulkan support!");
   }

   for (const auto& device : devices)
   {
      if (isDeviceSuitable (device))
      {
         physicalDevice = device;
         break;
      }
   }

   if (physicalDevice == VK_NULL_HANDLE)
   {
      throw std::runtime_error ("failed to find a suitable GPU!");
   }

}

bool HelloTriangleApplication::isDeviceSuitable (vk::PhysicalDevice device)
{
   QueueFamilyIndices indices = findQueueFamilies (device);

   bool extensionsSupported = checkDeviceExtensionSupport (device);

   bool swapChainAdequate = false;

   if (extensionsSupported)
   {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport (device);
      swapChainAdequate = !swapChainSupport.formats.empty () && !swapChainSupport.presentModes.empty ();

   }

   vk::PhysicalDeviceFeatures supportedFeatures = device.getFeatures ();

   return indices.isComplete () && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices HelloTriangleApplication::findQueueFamilies (vk::PhysicalDevice device)
{
   QueueFamilyIndices indices;

   std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties ();

   int i = 0;
   for (const auto& queueFamily : queueFamilies)
   {
      if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
      {
         indices.graphicsFamily = i;
      }

      vk::Bool32 presentSupport = false;
      device.getSurfaceSupportKHR (i, surface, &presentSupport);

      if (queueFamily.queueCount > 0 && presentSupport)
      {
         indices.presentFamily = i;
      }

      if (queueFamily.queueCount > 0 && (queueFamily.queueFlags & vk::QueueFlagBits::eTransfer) && !(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics))
      {
         indices.transferFamily = i;
      }

      if (indices.isComplete ())
      {
         break;
      }

      ++i;
   }

   return indices;
}

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport (vk::PhysicalDevice device)
{
   SwapChainSupportDetails details;

   details.capabilities = device.getSurfaceCapabilitiesKHR (surface);

   details.formats = device.getSurfaceFormatsKHR (surface);

   details.presentModes = device.getSurfacePresentModesKHR (surface);

   return details;
}

void HelloTriangleApplication::createSwapChain ()
{
   SwapChainSupportDetails swapChainSupport = querySwapChainSupport (physicalDevice);

   vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat (swapChainSupport.formats);
   vk::PresentModeKHR presentMode = chooseSwapPresentMode (swapChainSupport.presentModes);

   vk::Extent2D extent = chooseSwapExtent (swapChainSupport.capabilities);

   uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
   if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
   {
      imageCount = swapChainSupport.capabilities.maxImageCount;
   }

   vk::SwapchainCreateInfoKHR createInfo = {};

   createInfo.surface = surface;

   createInfo.minImageCount = imageCount;
   createInfo.imageFormat = surfaceFormat.format;
   createInfo.imageColorSpace = surfaceFormat.colorSpace;
   createInfo.imageExtent = extent;
   createInfo.imageArrayLayers = 1; //Always 1 for Non-Stereoscopic
   createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;

   QueueFamilyIndices indices = findQueueFamilies (physicalDevice);
   uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily, (uint32_t) indices.presentFamily, (uint32_t) indices.transferFamily};

   if (indices.graphicsFamily != indices.presentFamily)
   {
      createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
      createInfo.queueFamilyIndexCount = 3;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
   }
   else
   {
      createInfo.imageSharingMode = vk::SharingMode::eExclusive;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices = nullptr;
   }

   createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
   createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;

   createInfo.presentMode = presentMode;
   createInfo.clipped = VK_TRUE;
   createInfo.oldSwapchain = VK_NULL_HANDLE;

   if (device.createSwapchainKHR (&createInfo, nullptr, &swapChain) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create swap chain!");
   }

   //Retrieve Image Handles
   swapChainImages = device.getSwapchainImagesKHR (swapChain);
   swapChainImageFormat = surfaceFormat.format;
   swapChainExtent = extent;


}

vk::SurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat (const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
   if (availableFormats.size () == 1 && availableFormats[0].format == vk::Format::eUndefined)
   {
      return {vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear};
   }

   for (const auto& availableFormat : availableFormats)
   {
      if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
      {
         return availableFormat;
      }
   }

   //Could rank the available formats and choose the best but instead just take the first one
   return availableFormats[0];
}

vk::PresentModeKHR HelloTriangleApplication::chooseSwapPresentMode (const std::vector<vk::PresentModeKHR>& availablePresentModes)
{

   vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

   for (const auto& availablePresentMode : availablePresentModes)
   {
      if (availablePresentMode == vk::PresentModeKHR::eMailbox)
      {
         return availablePresentMode;
      }
      else if (availablePresentMode == vk::PresentModeKHR::eFifo)
      {
         bestMode = availablePresentMode;
      }
   }

   return bestMode;
}

vk::Extent2D HelloTriangleApplication::chooseSwapExtent (const vk::SurfaceCapabilitiesKHR & capabilities)
{
   if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max ())
   {
      return capabilities.currentExtent;
   }
   else
   {
      int width, height;
      glfwGetWindowSize (window, &width, &height);

      vk::Extent2D actualExtent = {static_cast<uint32_t> (width), static_cast<uint32_t> (height)};

      actualExtent.width = std::clamp (actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
      actualExtent.height = std::clamp (actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
      return actualExtent;
   }
}

void HelloTriangleApplication::createImageViews ()
{
   swapChainImageViews.resize (swapChainImages.size ());

   for (size_t i = 0; i < swapChainImages.size (); ++i)
   {
      swapChainImageViews[i] = createImageView (swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor);
   }
}

vk::ImageView HelloTriangleApplication::createImageView (vk::Image & image, vk::Format format, vk::ImageAspectFlags aspectFlags)
{
   vk::ImageViewCreateInfo viewInfo = {};
   viewInfo.image = image;

   viewInfo.viewType = vk::ImageViewType::e2D;
   viewInfo.format = format;

   viewInfo.components.r = vk::ComponentSwizzle::eIdentity;
   viewInfo.components.g = vk::ComponentSwizzle::eIdentity;
   viewInfo.components.b = vk::ComponentSwizzle::eIdentity;
   viewInfo.components.r = vk::ComponentSwizzle::eIdentity;

   viewInfo.subresourceRange.aspectMask = aspectFlags;
   viewInfo.subresourceRange.baseMipLevel = 0;
   viewInfo.subresourceRange.levelCount = 1;
   viewInfo.subresourceRange.baseArrayLayer = 0;
   viewInfo.subresourceRange.layerCount = 1;

   vk::ImageView imageView;
   if (device.createImageView (&viewInfo, nullptr, &imageView) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create image view!");
   }

   return imageView;
}

void HelloTriangleApplication::createRenderPass ()
{
   vk::AttachmentDescription colorAttachment = {};
   colorAttachment.format = swapChainImageFormat;
   colorAttachment.samples = vk::SampleCountFlagBits::e1;
   colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
   colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;

   colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
   colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;

   colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
   colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

   vk::AttachmentReference colorAttachmentRef = {};
   colorAttachmentRef.attachment = 0;
   colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

   vk::AttachmentDescription depthAttachment = {};
   depthAttachment.format = findDepthFormat ();
   depthAttachment.samples = vk::SampleCountFlagBits::e1;
   depthAttachment.loadOp = vk::AttachmentLoadOp::eClear;
   depthAttachment.storeOp = vk::AttachmentStoreOp::eDontCare;
   depthAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
   depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
   depthAttachment.initialLayout = vk::ImageLayout::eUndefined;
   depthAttachment.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

   vk::AttachmentReference depthAttachmentRef = {};
   depthAttachmentRef.attachment = 1;
   depthAttachmentRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;

   vk::SubpassDescription subpass = {};
   subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &colorAttachmentRef;
   subpass.pDepthStencilAttachment = &depthAttachmentRef;

   vk::SubpassDependency dependency = {};
   dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
   dependency.dstSubpass = 0;
   dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
   dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
   dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

   std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

   vk::RenderPassCreateInfo renderPassInfo = {};
   renderPassInfo.attachmentCount = static_cast<uint32_t> (attachments.size ());
   renderPassInfo.pAttachments = attachments.data ();
   renderPassInfo.subpassCount = 1;
   renderPassInfo.pSubpasses = &subpass;
   renderPassInfo.dependencyCount = 1;
   renderPassInfo.pDependencies = &dependency;

   if (device.createRenderPass (&renderPassInfo, nullptr, &renderPass) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create render pass!");
   }
}

void HelloTriangleApplication::createGraphicsPipeline ()
{
   auto vertShaderCode = readFile ("shaders/vert.spv");

   std::cout << "shaders/vert.spv read with size: " << vertShaderCode.size () << " bytes" << std::endl;

   auto fragShaderCode = readFile ("shaders/frag.spv");

   std::cout << "shaders/frag.spv read with size: " << fragShaderCode.size () << " bytes" << std::endl;

   vk::ShaderModule vertShaderModule = createShaderModule (vertShaderCode);
   vk::ShaderModule fragShaderModule = createShaderModule (fragShaderCode);


   vk::PipelineShaderStageCreateInfo vertShaderStageInfo = {};
   vertShaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;

   vertShaderStageInfo.module = vertShaderModule;
   vertShaderStageInfo.pName = "main";

   vk::PipelineShaderStageCreateInfo fragShaderStageInfo = {};
   fragShaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment;

   fragShaderStageInfo.module = fragShaderModule;
   fragShaderStageInfo.pName = "main";

   vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

   vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};

   auto bindingDescription = Vertex::getBindingDescription ();
   auto attributeDescription = Vertex::getAttributeDescriptions ();

   vertexInputInfo.vertexBindingDescriptionCount = 1;
   vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
   vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t> (attributeDescription.size ());
   vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data ();

   vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
   inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
   inputAssembly.primitiveRestartEnable = VK_FALSE;

   vk::Viewport viewport = {};
   viewport.x = 0.0f;
   viewport.y = 0.0f;

   viewport.width = (float) swapChainExtent.width;
   viewport.height = (float) swapChainExtent.height;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 1.0f;

   vk::Rect2D scissor = {};
   scissor.offset = {0, 0};
   scissor.extent = swapChainExtent;

   vk::PipelineViewportStateCreateInfo viewportState = {};
   viewportState.viewportCount = 1;
   viewportState.pViewports = &viewport;
   viewportState.scissorCount = 1;
   viewportState.pScissors = &scissor;

   vk::PipelineRasterizationStateCreateInfo rasterizer = {};
   rasterizer.depthClampEnable = VK_FALSE;
   rasterizer.rasterizerDiscardEnable = VK_FALSE;
   rasterizer.polygonMode = vk::PolygonMode::eFill;
   rasterizer.lineWidth = 1.0f;
   rasterizer.cullMode = vk::CullModeFlagBits::eBack;
   rasterizer.frontFace = vk::FrontFace::eCounterClockwise;
   rasterizer.depthBiasEnable = VK_FALSE;
   rasterizer.depthBiasConstantFactor = 0.0f;
   rasterizer.depthBiasClamp = 0.0f;
   rasterizer.depthBiasSlopeFactor = 0.0f;

   vk::PipelineMultisampleStateCreateInfo multisampling = {};
   multisampling.sampleShadingEnable = VK_FALSE;
   multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
   multisampling.minSampleShading = 1.0f;
   multisampling.pSampleMask = nullptr;
   multisampling.alphaToCoverageEnable = VK_FALSE;
   multisampling.alphaToOneEnable = VK_FALSE;

   vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
   colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
   colorBlendAttachment.blendEnable = VK_FALSE;
   colorBlendAttachment.srcColorBlendFactor = vk::BlendFactor::eOne;
   colorBlendAttachment.dstColorBlendFactor = vk::BlendFactor::eZero;
   colorBlendAttachment.colorBlendOp = vk::BlendOp::eAdd;
   colorBlendAttachment.srcAlphaBlendFactor = vk::BlendFactor::eOne;
   colorBlendAttachment.dstAlphaBlendFactor = vk::BlendFactor::eZero;
   colorBlendAttachment.alphaBlendOp = vk::BlendOp::eAdd;

   vk::PipelineColorBlendStateCreateInfo colorBlending = {};
   colorBlending.logicOpEnable = VK_FALSE;
   colorBlending.logicOp = vk::LogicOp::eCopy;
   colorBlending.attachmentCount = 1;
   colorBlending.pAttachments = &colorBlendAttachment;
   colorBlending.blendConstants[0] = 0.0f;
   colorBlending.blendConstants[1] = 0.0f;
   colorBlending.blendConstants[2] = 0.0f;
   colorBlending.blendConstants[3] = 0.0f;

   vk::PipelineDepthStencilStateCreateInfo depthStencil = {};
   depthStencil.depthTestEnable = VK_TRUE;
   depthStencil.depthWriteEnable = VK_TRUE;
   depthStencil.depthCompareOp = vk::CompareOp::eLess;
   depthStencil.depthBoundsTestEnable = VK_FALSE;
   depthStencil.stencilTestEnable = VK_FALSE;

   vk::DynamicState dynamicStates[] = {vk::DynamicState::eViewport, vk::DynamicState::eLineWidth};
   vk::PipelineDynamicStateCreateInfo dynamicState = {};
   dynamicState.dynamicStateCount = 2;
   dynamicState.pDynamicStates = dynamicStates;

   vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
   pipelineLayoutInfo.setLayoutCount = 1;
   pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
   pipelineLayoutInfo.pushConstantRangeCount = 0;
   pipelineLayoutInfo.pPushConstantRanges = nullptr;

   if (device.createPipelineLayout (&pipelineLayoutInfo, nullptr, &pipelineLayout) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create the pipeline layout!");
   }

   vk::GraphicsPipelineCreateInfo pipelineInfo = {};
   pipelineInfo.stageCount = 2;
   pipelineInfo.pStages = shaderStages;
   pipelineInfo.pVertexInputState = &vertexInputInfo;
   pipelineInfo.pInputAssemblyState = &inputAssembly;
   pipelineInfo.pViewportState = &viewportState;
   pipelineInfo.pRasterizationState = &rasterizer;
   pipelineInfo.pMultisampleState = &multisampling;
   pipelineInfo.pDepthStencilState = nullptr;
   pipelineInfo.pColorBlendState = &colorBlending;
   pipelineInfo.pDepthStencilState = &depthStencil;
   pipelineInfo.pDynamicState = nullptr;

   pipelineInfo.layout = pipelineLayout;
   pipelineInfo.renderPass = renderPass;
   pipelineInfo.subpass = 0;

   if (device.createGraphicsPipelines (VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create graphics pipeline!");
   }

   device.destroyShaderModule (fragShaderModule, nullptr);
   device.destroyShaderModule (vertShaderModule, nullptr);
}

vk::ShaderModule HelloTriangleApplication::createShaderModule (const std::vector<char>& code)
{
   vk::ShaderModuleCreateInfo createInfo = {};

   createInfo.codeSize = code.size ();
   createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data ());

   vk::ShaderModule shaderModule;
   if (device.createShaderModule (&createInfo, nullptr, &shaderModule) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create shader module!");
   }

   return shaderModule;
}

void HelloTriangleApplication::createFramebuffers ()
{
   swapChainFramebuffers.resize (swapChainImageViews.size ());

   for (size_t i = 0; i < swapChainImageViews.size (); ++i)
   {
      std::array<vk::ImageView, 2> attachments = {swapChainImageViews[i], depthImageView};

      vk::FramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = static_cast<uint32_t> (attachments.size ());
      framebufferInfo.pAttachments = attachments.data ();
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (device.createFramebuffer (&framebufferInfo, nullptr, &swapChainFramebuffers[i]) != vk::Result::eSuccess)
      {
         throw std::runtime_error ("failed to create framebuffer!");
      }
   }
}

void HelloTriangleApplication::createCommandPool ()
{
   QueueFamilyIndices queueFamilyIndices = findQueueFamilies (physicalDevice);

   vk::CommandPoolCreateInfo commandPoolGraphicsInfo = {};
   commandPoolGraphicsInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;

   if (device.createCommandPool (&commandPoolGraphicsInfo, nullptr, &commandPoolGraphics) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create command pool!");
   }

   vk::CommandPoolCreateInfo commandPoolTransferInfo = {};
   commandPoolTransferInfo.queueFamilyIndex = queueFamilyIndices.transferFamily;

   if (device.createCommandPool (&commandPoolTransferInfo, nullptr, &commandPoolTransfer) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create command pool!");
   }
}

void HelloTriangleApplication::createCommandBuffers ()
{
   commandBuffers.resize (swapChainFramebuffers.size ());

   vk::CommandBufferAllocateInfo allocInfo = {};
   allocInfo.commandPool = commandPoolGraphics;
   allocInfo.level = vk::CommandBufferLevel::ePrimary;
   allocInfo.commandBufferCount = (uint32_t) commandBuffers.size ();

   if (device.allocateCommandBuffers (&allocInfo, commandBuffers.data ()) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to allocate command buffers!");
   }

   for (size_t i = 0; i < commandBuffers.size (); ++i)
   {
      vk::CommandBufferBeginInfo beginInfo = {};
      beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;
      beginInfo.pInheritanceInfo = nullptr;

      commandBuffers[i].begin (&beginInfo);

      vk::RenderPassBeginInfo renderPassInfo = {};
      renderPassInfo.renderPass = renderPass;
      renderPassInfo.framebuffer = swapChainFramebuffers[i];

      renderPassInfo.renderArea.offset = {0, 0};
      renderPassInfo.renderArea.extent = swapChainExtent;

      std::array<vk::ClearValue, 2> clearValues = {};
      vk::ClearColorValue clearColValue = {};
      clearColValue.setFloat32 ({0.0f, 0.0f, 0.0f, 1.0f});
      clearValues[0].setColor (clearColValue);

      clearValues[1].depthStencil = {1.0f, 0};

      renderPassInfo.clearValueCount = static_cast<uint32_t> (clearValues.size ());
      renderPassInfo.pClearValues = clearValues.data ();

      commandBuffers[i].beginRenderPass (&renderPassInfo, vk::SubpassContents::eInline);

      commandBuffers[i].bindPipeline (vk::PipelineBindPoint::eGraphics, graphicsPipeline);

      commandBuffers[i].bindDescriptorSets (vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

      vk::Buffer vertexBuffers[] = {vertexBuffer};
      vk::DeviceSize offsets[] = {0};
      commandBuffers[i].bindVertexBuffers (0, 1, vertexBuffers, offsets);

      commandBuffers[i].bindIndexBuffer (indexBuffer, 0, vk::IndexType::eUint32);

      commandBuffers[i].drawIndexed (static_cast<uint32_t>(indices.size ()), 1, 0, 0, 0);

      commandBuffers[i].endRenderPass ();

      commandBuffers[i].end ();
   }

}

void HelloTriangleApplication::createSemaphores ()
{
   vk::SemaphoreCreateInfo semaphoreInfo = {};

   if (device.createSemaphore (&semaphoreInfo, nullptr, &imageAvailableSemaphore) != vk::Result::eSuccess
       || device.createSemaphore (&semaphoreInfo, nullptr, &renderFinishedSemaphore) != vk::Result::eSuccess
       )
   {
      throw std::runtime_error ("failed to create semaphores!");
   }
}


void HelloTriangleApplication::createLogicalDevice ()
{
   QueueFamilyIndices indices = findQueueFamilies (physicalDevice);

   std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
   std::set<int> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily, indices.transferFamily};

   float queuePriority = 1.0f;

   for (int queueFamily : uniqueQueueFamilies)
   {
      vk::DeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back (queueCreateInfo);
   }

   vk::PhysicalDeviceFeatures deviceFeatures = {};
   deviceFeatures.samplerAnisotropy = VK_TRUE;

   vk::DeviceCreateInfo createInfo = {};

   createInfo.queueCreateInfoCount = static_cast<uint32_t> (queueCreateInfos.size ());
   createInfo.pQueueCreateInfos = queueCreateInfos.data ();

   createInfo.pEnabledFeatures = &deviceFeatures;

   createInfo.enabledExtensionCount = static_cast<uint32_t> (deviceExtensions.size ());
   createInfo.ppEnabledExtensionNames = deviceExtensions.data ();

   if (enableValidationLayers)
   {
      createInfo.enabledLayerCount = static_cast<uint32_t> (validationLayers.size ());
      createInfo.ppEnabledLayerNames = validationLayers.data ();
   }
   else
   {
      createInfo.enabledLayerCount = 0;
   }

   if (physicalDevice.createDevice (&createInfo, nullptr, &device) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create logical device!");
   }

   device.getQueue (indices.graphicsFamily, 0, &graphicsQueue);
   device.getQueue (indices.presentFamily, 0, &presentQueue);
   device.getQueue (indices.transferFamily, 0, &transferQueue);
}

void HelloTriangleApplication::mainLoop ()
{
   while (!glfwWindowShouldClose (window))
   {
      glfwPollEvents ();

      updateUniformBuffer ();
      drawFrame ();
   }

   device.waitIdle ();
}

void HelloTriangleApplication::updateUniformBuffer ()
{
   static auto startTime = std::chrono::high_resolution_clock::now ();

   auto currentTime = std::chrono::high_resolution_clock::now ();

   float time = std::chrono::duration<float, std::chrono::seconds::period> (currentTime - startTime).count ();
   UniformBufferObject ubo = {};
   ubo.model = glm::rotate (glm::mat4 (1.0f), time * glm::radians (90.0f), glm::vec3 (0.0f, 0.0f, 1.0f));

   ubo.view = glm::lookAt (glm::vec3 (2.0f, 2.0f, 2.0f), glm::vec3 (0.0f, 0.0f, 0.0f), glm::vec3 (0.0f, 0.0f, 1.0f));

   ubo.proj = glm::perspective (glm::radians (45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
   ubo.proj[1][1] *= -1; //glm is orignally designed for OpenGL which inverts its y-coordinate so we need to flip it

   void* data;
   data = device.mapMemory (uniformBufferMemory, 0, sizeof (ubo));
   memcpy (data, &ubo, sizeof (ubo));

   device.unmapMemory (uniformBufferMemory);
}

void HelloTriangleApplication::drawFrame ()
{
   uint32_t imageIndex;
   vk::Result result = device.acquireNextImageKHR (swapChain, std::numeric_limits<uint64_t>::max (), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

   if (result == vk::Result::eErrorOutOfDateKHR)
   {
      recreateSwapChain ();
      return;
   }
   else if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
   {
      throw std::runtime_error ("failed to acquire swap chain image!");
   }

   vk::SubmitInfo submitInfo = {};

   vk::Semaphore waitSemaphores[] = {imageAvailableSemaphore};
   vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};

   submitInfo.waitSemaphoreCount = 1;
   submitInfo.pWaitSemaphores = waitSemaphores;
   submitInfo.pWaitDstStageMask = waitStages;

   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

   vk::Semaphore signalSemaphores[] = {renderFinishedSemaphore};
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = signalSemaphores;

   if (graphicsQueue.submit (1, &submitInfo, VK_NULL_HANDLE) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to submit draw command buffer!");
   }

   vk::PresentInfoKHR presentInfo = {};
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = signalSemaphores;

   vk::SwapchainKHR swapChains[] = {swapChain};
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = swapChains;
   presentInfo.pImageIndices = &imageIndex;
   presentInfo.pResults = nullptr;


   result = presentQueue.presentKHR (&presentInfo);

   if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR)
   {
      recreateSwapChain ();
   }
   else if (result != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to present swap chain image!");
   }

   presentQueue.waitIdle ();
}

void HelloTriangleApplication::recreateSwapChain ()
{
   device.waitIdle ();

   cleanupSwapChain ();

   createSwapChain ();
   createImageViews ();
   createRenderPass ();
   createGraphicsPipeline ();
   createDepthResources ();
   createFramebuffers ();
   createCommandBuffers ();
}

void HelloTriangleApplication::cleanupSwapChain ()
{
   device.destroyImageView (depthImageView, nullptr);
   device.destroyImage (depthImage, nullptr);
   device.freeMemory (depthImageMemory, nullptr);

   for (auto swapChainFramebuffer : swapChainFramebuffers)
   {
      device.destroyFramebuffer (swapChainFramebuffer, nullptr);
   }

   device.freeCommandBuffers (commandPoolGraphics, static_cast<uint32_t> (commandBuffers.size ()), commandBuffers.data ());

   device.destroyPipeline (graphicsPipeline, nullptr);

   device.destroyPipelineLayout (pipelineLayout, nullptr);

   device.destroyRenderPass (renderPass, nullptr);

   for (auto swapChainImageView : swapChainImageViews)
   {
      device.destroyImageView (swapChainImageView, nullptr);
   }

   device.destroySwapchainKHR (swapChain, nullptr);
}

void HelloTriangleApplication::createVertexBuffer ()
{
   vk::DeviceSize bufferSize = sizeof (vertices[0]) * vertices.size ();

   vk::Buffer stagingBuffer;
   vk::DeviceMemory stagingBufferMemory;

   createBuffer (bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

   void* data;
   data = device.mapMemory (stagingBufferMemory, 0, bufferSize);
   memcpy (data, vertices.data (), (size_t) bufferSize);
   device.unmapMemory (stagingBufferMemory);

   createBuffer (bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, vertexBuffer, vertexBufferMemory);
   copyBuffer (stagingBuffer, vertexBuffer, bufferSize);

   device.destroyBuffer (stagingBuffer, nullptr);
   device.freeMemory (stagingBufferMemory, nullptr);

}

void HelloTriangleApplication::createIndexBuffer ()
{
   vk::DeviceSize bufferSize = sizeof (indices[0]) * indices.size ();

   vk::Buffer stagingBuffer;
   vk::DeviceMemory stagingBufferMemory;

   createBuffer (bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

   void* data;
   data = device.mapMemory (stagingBufferMemory, 0, bufferSize);
   memcpy (data, indices.data (), (size_t) bufferSize);
   device.unmapMemory (stagingBufferMemory);

   createBuffer (bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal, indexBuffer, indexBufferMemory);
   copyBuffer (stagingBuffer, indexBuffer, bufferSize);

   device.destroyBuffer (stagingBuffer, nullptr);
   device.freeMemory (stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createUniformBuffer ()
{
   vk::DeviceSize  bufferSize = sizeof (UniformBufferObject);
   createBuffer (bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, uniformBuffer, uniformBufferMemory);
}

void HelloTriangleApplication::createDescriptorPool ()
{
   std::array<vk::DescriptorPoolSize, 2> poolSize = {};
   poolSize[0].type = vk::DescriptorType::eUniformBuffer;
   poolSize[0].descriptorCount = 1;
   poolSize[1].type = vk::DescriptorType::eCombinedImageSampler;
   poolSize[1].descriptorCount = 1;

   vk::DescriptorPoolCreateInfo poolInfo = {};
   poolInfo.poolSizeCount = poolSize.size ();
   poolInfo.pPoolSizes = poolSize.data ();
   poolInfo.maxSets = 1;

   if (device.createDescriptorPool (&poolInfo, nullptr, &descriptorPool) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create descriptor pool!");
   }
}

void HelloTriangleApplication::createDescriptorSet ()
{
   vk::DescriptorSetLayout layouts[] = {descriptorSetLayout};
   vk::DescriptorSetAllocateInfo allocInfo = {};
   allocInfo.descriptorPool = descriptorPool;
   allocInfo.descriptorSetCount = 1;
   allocInfo.pSetLayouts = layouts;

   if (device.allocateDescriptorSets (&allocInfo, &descriptorSet) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to allocate descriptor set!");
   }

   vk::DescriptorBufferInfo bufferInfo = {};
   bufferInfo.buffer = uniformBuffer;
   bufferInfo.offset = 0;
   bufferInfo.range = sizeof (UniformBufferObject);

   vk::DescriptorImageInfo imageInfo = {};
   imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
   imageInfo.imageView = textureImageView;
   imageInfo.sampler = textureSampler;

   std::array<vk::WriteDescriptorSet, 2> descriptorWrite = {};
   descriptorWrite[0].dstSet = descriptorSet;
   descriptorWrite[0].dstBinding = 0;
   descriptorWrite[0].dstArrayElement = 0;
   descriptorWrite[0].descriptorType = vk::DescriptorType::eUniformBuffer;
   descriptorWrite[0].descriptorCount = 1;
   descriptorWrite[0].pBufferInfo = &bufferInfo;

   descriptorWrite[1].dstSet = descriptorSet;
   descriptorWrite[1].dstBinding = 1;
   descriptorWrite[1].dstArrayElement = 0;
   descriptorWrite[1].descriptorType = vk::DescriptorType::eCombinedImageSampler;
   descriptorWrite[1].descriptorCount = 1;
   descriptorWrite[1].pImageInfo = &imageInfo;

   device.updateDescriptorSets (static_cast<uint32_t>(descriptorWrite.size ()), descriptorWrite.data (), 0, nullptr);
}


void HelloTriangleApplication::createBuffer (vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)
{
   vk::BufferCreateInfo bufferInfo = {};

   bufferInfo.size = size;
   bufferInfo.usage = usage;
   bufferInfo.sharingMode = vk::SharingMode::eConcurrent;

   QueueFamilyIndices indices = findQueueFamilies (physicalDevice);
   uint32_t queueFamilies[] = {static_cast<uint32_t> (indices.graphicsFamily), static_cast<uint32_t> (indices.transferFamily)};
   bufferInfo.queueFamilyIndexCount = 2;
   bufferInfo.pQueueFamilyIndices = queueFamilies;


   if (device.createBuffer (&bufferInfo, nullptr, &buffer) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create buffer!");
   }

   vk::MemoryRequirements memRequirements;
   device.getBufferMemoryRequirements (buffer, &memRequirements);

   vk::MemoryAllocateInfo allocInfo = {};
   allocInfo.allocationSize = memRequirements.size;
   allocInfo.memoryTypeIndex = findMemoryType (memRequirements.memoryTypeBits, properties);

   if (device.allocateMemory (&allocInfo, nullptr, &bufferMemory) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to allocate buffer memory!");
   }

   device.bindBufferMemory (buffer, bufferMemory, 0);
}

void HelloTriangleApplication::copyBuffer (vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
   vk::CommandBuffer commandBuffer = beginSingleTimeCommands (commandPoolTransfer);

   vk::BufferCopy copyRegion = {};
   copyRegion.size = size;

   commandBuffer.copyBuffer (srcBuffer, dstBuffer, 1, &copyRegion);

   endSingleTimeCommands (commandBuffer, commandPoolTransfer, transferQueue);
}

void HelloTriangleApplication::createDescriptorSetLayout ()
{
   vk::DescriptorSetLayoutBinding uboLayoutBinding = {};
   uboLayoutBinding.binding = 0;
   uboLayoutBinding.descriptorType = vk::DescriptorType::eUniformBuffer;
   uboLayoutBinding.descriptorCount = 1;

   uboLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eVertex;
   uboLayoutBinding.pImmutableSamplers = nullptr;

   vk::DescriptorSetLayoutBinding samplerLayoutBinding = {};
   samplerLayoutBinding.binding = 1;
   samplerLayoutBinding.descriptorCount = 1;
   samplerLayoutBinding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
   samplerLayoutBinding.pImmutableSamplers = nullptr;
   samplerLayoutBinding.stageFlags = vk::ShaderStageFlagBits::eFragment;

   std::array<vk::DescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};

   vk::DescriptorSetLayoutCreateInfo layoutInfo = {};
   layoutInfo.bindingCount = bindings.size ();
   layoutInfo.pBindings = bindings.data ();

   if (device.createDescriptorSetLayout (&layoutInfo, nullptr, &descriptorSetLayout) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create descriptor set layout!");
   }
}

void HelloTriangleApplication::createTextureImage ()
{
   int texWidth, texHeight, texChannels;

   stbi_uc* pixels = stbi_load (TEXTURE_PATH.c_str (), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
   vk::DeviceSize imageSize = texWidth * texHeight * 4; //4 bytes per pixel

   if (!pixels)
   {
      std::runtime_error ("failed to load texture image!");
   }

   vk::Buffer stagingBuffer;
   vk::DeviceMemory stagingBufferMemory;

   createBuffer (imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer, stagingBufferMemory);

   void* data;
   data = device.mapMemory (stagingBufferMemory, 0, imageSize);
   memcpy (data, pixels, static_cast<size_t>(imageSize));
   device.unmapMemory (stagingBufferMemory);

   stbi_image_free (pixels);

   createImage (texWidth, texHeight, vk::Format::eR8G8B8A8Unorm, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, textureImage, textureImageMemory);

   transitionImageLayout (textureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, commandPoolGraphics, graphicsQueue);
   copyBufferToImage (stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
   transitionImageLayout (textureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, commandPoolGraphics, graphicsQueue);

   device.destroyBuffer (stagingBuffer, nullptr);
   device.freeMemory (stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createImage (uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image & image, vk::DeviceMemory & imageMemory)
{
   vk::ImageCreateInfo imageInfo = {};
   imageInfo.imageType = vk::ImageType::e2D;
   imageInfo.extent.width = static_cast<uint32_t> (width);
   imageInfo.extent.height = static_cast<uint32_t> (height);
   imageInfo.extent.depth = 1;
   imageInfo.mipLevels = 1;
   imageInfo.arrayLayers = 1;

   imageInfo.format = format;

   imageInfo.tiling = tiling;
   imageInfo.initialLayout = vk::ImageLayout::eUndefined;;

   imageInfo.usage = usage;
   imageInfo.samples = vk::SampleCountFlagBits::e1;
   imageInfo.sharingMode = vk::SharingMode::eExclusive;


   if (device.createImage (&imageInfo, nullptr, &image) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create image!");
   }

   vk::MemoryRequirements memRequirements;
   device.getImageMemoryRequirements (image, &memRequirements);

   vk::MemoryAllocateInfo allocInfo = {};
   allocInfo.allocationSize = memRequirements.size;
   allocInfo.memoryTypeIndex = findMemoryType (memRequirements.memoryTypeBits, properties);

   if (device.allocateMemory (&allocInfo, nullptr, &imageMemory) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to allocate image memory!");
   }

   device.bindImageMemory (image, imageMemory, 0);
}

void HelloTriangleApplication::transitionImageLayout (vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandPool commandPool, vk::Queue queue)
{
   vk::CommandBuffer commandBuffer = beginSingleTimeCommands (commandPool);

   vk::ImageMemoryBarrier barrier = {};
   barrier.oldLayout = oldLayout;
   barrier.newLayout = newLayout;

   barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
   barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

   barrier.image = image;

   if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
   {
      barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

      if (hasStencilComponent (format))
      {
         barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
      }
   }
   else
   {
      barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
   }

   barrier.subresourceRange.baseMipLevel = 0;
   barrier.subresourceRange.levelCount = 1;
   barrier.subresourceRange.baseArrayLayer = 0;
   barrier.subresourceRange.layerCount = 1;

   vk::PipelineStageFlags sourceStage;
   vk::PipelineStageFlags destinationStage;

   if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
   {
      barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

      sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
      destinationStage = vk::PipelineStageFlagBits::eTransfer;
   }
   else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
   {
      barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
      barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

      sourceStage = vk::PipelineStageFlagBits::eTransfer;
      destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
   }
   else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
   {
      barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

      sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
      destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
   }
   else
   {
      throw std::invalid_argument ("unsupported layout transition!");
   }

   commandBuffer.pipelineBarrier (sourceStage, destinationStage, vk::DependencyFlags (), 0, nullptr, 0, nullptr, 1, &barrier);


   endSingleTimeCommands (commandBuffer, commandPool, queue);
}

void HelloTriangleApplication::copyBufferToImage (vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
   vk::CommandBuffer commandBuffer = beginSingleTimeCommands (commandPoolTransfer);

   vk::BufferImageCopy region = {};
   region.bufferOffset = 0;
   region.bufferRowLength = 0;
   region.bufferImageHeight = 0;

   region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
   region.imageSubresource.mipLevel = 0;
   region.imageSubresource.baseArrayLayer = 0;
   region.imageSubresource.layerCount = 1;

   region.imageOffset = {0, 0, 0};
   region.imageExtent = {width, height, 1};

   commandBuffer.copyBufferToImage (buffer, image, vk::ImageLayout::eTransferDstOptimal, 1, &region);

   endSingleTimeCommands (commandBuffer, commandPoolTransfer, transferQueue);
}

void HelloTriangleApplication::createTextureImageView ()
{
   textureImageView = createImageView (textureImage, vk::Format::eR8G8B8A8Unorm, vk::ImageAspectFlagBits::eColor);
}

void HelloTriangleApplication::createTextureSampler ()
{
   vk::SamplerCreateInfo samplerInfo = {};
   samplerInfo.magFilter = vk::Filter::eLinear;
   samplerInfo.minFilter = vk::Filter::eLinear;

   samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
   samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
   samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;

   samplerInfo.anisotropyEnable = VK_TRUE;
   samplerInfo.maxAnisotropy = 16;

   samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;

   samplerInfo.unnormalizedCoordinates = VK_FALSE;

   samplerInfo.compareEnable = VK_FALSE;
   samplerInfo.compareOp = vk::CompareOp::eAlways;

   samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
   samplerInfo.mipLodBias = 0.0f;
   samplerInfo.minLod = 0.0f;
   samplerInfo.maxLod = 0.0f;

   if (device.createSampler (&samplerInfo, nullptr, &textureSampler) != vk::Result::eSuccess)
   {
      throw std::runtime_error ("failed to create texture sampler!");
   }
}

void HelloTriangleApplication::createDepthResources ()
{
   vk::Format depthFormat = findDepthFormat ();

   createImage (swapChainExtent.width, swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);
   depthImageView = createImageView (depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);

   transitionImageLayout (depthImage, depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal, commandPoolGraphics, graphicsQueue);

}

vk::Format HelloTriangleApplication::findSupportedFormat (const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
   for (auto format : candidates)
   {
      vk::FormatProperties props;
      physicalDevice.getFormatProperties (format, &props);

      if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)
      {
         return format;
      }
      else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features)
      {
         return format;
      }

   }

   throw std::runtime_error ("failed to find supported format!");

}

vk::Format HelloTriangleApplication::findDepthFormat ()
{
   return findSupportedFormat (
      {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
      vk::ImageTiling::eOptimal,
      vk::FormatFeatureFlagBits::eDepthStencilAttachment
   );
}

bool HelloTriangleApplication::hasStencilComponent (vk::Format format)
{
   return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

void HelloTriangleApplication::loadModel ()
{
   tinyobj::attrib_t attrib;
   std::vector<tinyobj::shape_t> shapes;
   std::vector<tinyobj::material_t> materials;
   std::string err;

   if (!tinyobj::LoadObj (&attrib, &shapes, &materials, &err, MODEL_PATH.c_str ()))
   {
      throw std::runtime_error (err);
   }
   std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

   for (const auto& shape : shapes)
   {
      for (const auto& index : shape.mesh.indices)
      {
         Vertex vertex = {};

         vertex.pos = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2]
         };

         vertex.texCoord = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            1.0f - attrib.texcoords[2 * index.texcoord_index + 1] //Origin of texture was assumed bottom left, but Vulkan assumes top-left
         };

         vertex.color = {1.0f, 1.0f, 1.0f};

         if (uniqueVertices.count (vertex) == 0)
         {
            uniqueVertices[vertex] = static_cast<uint32_t> (vertices.size ());
            vertices.push_back (vertex);
         }

         indices.push_back (uniqueVertices[vertex]);
      }
   }

   std::cout << "Model loaded." << std::endl;
}

uint32_t HelloTriangleApplication::findMemoryType (uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
   vk::PhysicalDeviceMemoryProperties memProperties;
   physicalDevice.getMemoryProperties (&memProperties);

   for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
   {
      if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties))
      {
         return i;
      }
   }

   throw std::runtime_error ("failed to find suitable memory type!");
}

vk::CommandBuffer HelloTriangleApplication::beginSingleTimeCommands (vk::CommandPool & commandPool)
{
   vk::CommandBufferAllocateInfo allocInfo = {};
   allocInfo.level = vk::CommandBufferLevel::ePrimary;
   allocInfo.commandPool = commandPool;
   allocInfo.commandBufferCount = 1;

   vk::CommandBuffer commandBuffer;
   device.allocateCommandBuffers (&allocInfo, &commandBuffer);

   vk::CommandBufferBeginInfo beginInfo = {};
   beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

   commandBuffer.begin (&beginInfo);

   return commandBuffer;
}

void HelloTriangleApplication::endSingleTimeCommands (vk::CommandBuffer commandBuffer, vk::CommandPool commandPool, vk::Queue queue)
{
   commandBuffer.end ();

   vk::SubmitInfo submitInfo = {};
   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &commandBuffer;

   queue.submit (1, &submitInfo, VK_NULL_HANDLE);
   queue.waitIdle ();

   device.freeCommandBuffers (commandPool, 1, &commandBuffer);
}

void HelloTriangleApplication::cleanup ()
{
   cleanupSwapChain ();

   device.destroySampler (textureSampler, nullptr);

   device.destroyImageView (textureImageView, nullptr);

   device.destroyImage (textureImage, nullptr);
   device.freeMemory (textureImageMemory, nullptr);

   device.destroyDescriptorPool (descriptorPool, nullptr);

   device.destroyDescriptorSetLayout (descriptorSetLayout, nullptr);

   device.destroyBuffer (uniformBuffer, nullptr);
   device.freeMemory (uniformBufferMemory, nullptr);

   device.destroyBuffer (indexBuffer, nullptr);
   device.freeMemory (indexBufferMemory, nullptr);

   device.destroyBuffer (vertexBuffer, nullptr);
   device.freeMemory (vertexBufferMemory, nullptr);

   device.destroySemaphore (renderFinishedSemaphore, nullptr);
   device.destroySemaphore (imageAvailableSemaphore, nullptr);

   device.destroyCommandPool (commandPoolGraphics, nullptr);
   device.destroyCommandPool (commandPoolTransfer, nullptr);

   device.destroy (nullptr);

   DestroyDebugReportCallbackEXT (
      VkInstance (instance),
      static_cast<VkDebugReportCallbackEXT>(callback),
      nullptr);

   instance.destroySurfaceKHR (surface, nullptr);

   instance.destroy (nullptr);

   glfwDestroyWindow (window);

   glfwTerminate ();
}

VkResult HelloTriangleApplication::CreateDebugReportCallbackEXT (VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback)
{
   auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr (instance,
                                                                           "vkCreateDebugReportCallbackEXT");
   if (func != nullptr)
   {
      return func (instance, pCreateInfo, pAllocator, pCallback);
   }
   else
   {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
   }
}

void HelloTriangleApplication::DestroyDebugReportCallbackEXT (VkInstance instance, VkDebugReportCallbackEXT callback, 
                                                              const VkAllocationCallbacks* pAllocator)
{
   auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr (instance,
                                                                            "vkDestroyDebugReportCallbackEXT");
   if (func != nullptr)
   {
      func (instance, callback, pAllocator);
   }
}

bool HelloTriangleApplication::checkDeviceExtensionSupport (vk::PhysicalDevice device)
{
   auto availableExtensions = device.enumerateDeviceExtensionProperties ();

   std::set<std::string> requiredExtensions (deviceExtensions.begin (), deviceExtensions.end ());

   for (const auto& extension : availableExtensions)
   {
      requiredExtensions.erase (extension.extensionName);
   }

   return requiredExtensions.empty ();
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions ()
{
   std::vector<const char *> extensions;

   getRequiredGlfwExtensions (extensions);

   if (enableValidationLayers)
   {
      extensions.push_back (VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
   }

   return extensions;
}

void HelloTriangleApplication::checkValidationLayerSupport ()
{
   auto availableLayers = vk::enumerateInstanceLayerProperties ();

   std::vector<std::string> MissingValidationLayers;
   for (const auto& validationLayer : validationLayers)
   {
      auto matchExtension = [&] (const vk::LayerProperties& layerProp) { return strcmp (layerProp.layerName, validationLayer) == 0; };

      auto findIfRequiredExtensionSupported = std::find_if (std::begin (availableLayers), std::end (availableLayers), matchExtension);

      if (findIfRequiredExtensionSupported == std::end (availableLayers))
      {
         MissingValidationLayers.emplace_back (validationLayer);
      }
   }

   if (!MissingValidationLayers.empty ())
   {
      throw std::runtime_error ("Validation layers requested, but not available! Missing: \r\n\t"
                                + std::accumulate (std::next (MissingValidationLayers.begin ()), MissingValidationLayers.end (), MissingValidationLayers[0],
                                                   [] (std::string accum, std::string next) { return accum + "\r\n\t" + next; }));
   }
}

void HelloTriangleApplication::PrintSupportedValidationLayers ()
{
   auto availableLayers = vk::enumerateInstanceLayerProperties ();

   std::cout << "available validation layers: " << std::endl;

   for (const auto& layer : availableLayers)
   {
      std::cout << "\t" << layer.layerName << std::endl;
   }
}

void HelloTriangleApplication::checkInstanceExtensionSupport (std::vector<const char *> RequiredExtensions)
{
   auto SupportedExtensions = vk::enumerateInstanceExtensionProperties (nullptr);

   std::vector<std::string> MissingExtensions;

   for (const auto& RequiredExtension : RequiredExtensions)
   {
      auto matchExtension = [&] (const vk::ExtensionProperties& extensionProp) { return strcmp (extensionProp.extensionName, RequiredExtension) == 0; };

      auto findIfRequiredExtensionSupported = std::find_if (std::begin (SupportedExtensions), std::end (SupportedExtensions), matchExtension);

      if (findIfRequiredExtensionSupported == std::end (SupportedExtensions))
      {
         MissingExtensions.emplace_back (RequiredExtension);
      }
   }
   if (!MissingExtensions.empty ())
   {
      throw std::runtime_error ("A required extension is not supported! Missing: \r\n\t"
                                + std::accumulate (std::next (MissingExtensions.begin ()), MissingExtensions.end (), MissingExtensions[0],
                                                   [] (std::string accum, std::string next) { return accum + "\r\n\t" + next; }));
   }
}

void HelloTriangleApplication::PrintSupportedInstanceExtensions ()
{
   auto extensions = vk::enumerateInstanceExtensionProperties (nullptr);

   std::cout << "available extensions: " << std::endl;

   for (const auto& extension : extensions)
   {
      std::cout << "\t" << extension.extensionName << std::endl;
   }
}

void HelloTriangleApplication::PrintAvailablePhysicalDevices (const vk::Instance instance)
{
   auto devices = instance.enumeratePhysicalDevices ();

   std::cout << "available physical devices: " << std::endl;

   for (const auto& device : devices)
   {
      vk::PhysicalDeviceProperties deviceProperties = device.getProperties ();

      std::cout << "\t" << deviceProperties.deviceName << std::endl;
   }
}

vk::VertexInputBindingDescription Vertex::getBindingDescription ()
{
   vk::VertexInputBindingDescription bindingDescription = {};

   bindingDescription.binding = 0;
   bindingDescription.stride = sizeof (Vertex);
   bindingDescription.inputRate = vk::VertexInputRate::eVertex;

   return bindingDescription;
}

std::array<vk::VertexInputAttributeDescription, 3> Vertex::getAttributeDescriptions ()
{
   std::array<vk::VertexInputAttributeDescription, 3> attributeDescriptions = {};

   attributeDescriptions[0].binding = 0;
   attributeDescriptions[0].location = 0;
   attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
   attributeDescriptions[0].offset = offsetof (Vertex, pos);

   attributeDescriptions[1].binding = 0;
   attributeDescriptions[1].location = 1;
   attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
   attributeDescriptions[1].offset = offsetof (Vertex, color);

   attributeDescriptions[2].binding = 0;
   attributeDescriptions[2].location = 2;
   attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
   attributeDescriptions[2].offset = offsetof (Vertex, texCoord);

   return attributeDescriptions;
}
