#include "HelloTriangleApplication.h"

#include <functional>
#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>
#include <fstream>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
   const bool enableValidationLayers = false
#else
   const bool enableValidationLayers = true;
#endif

const std::vector<Vertex> vertices = {
   {{0.0f, -0.5}, {1.0f,1.0f,1.0f}},
   {{0.5, 0.5f},{0.0f,1.0f,0.0f}},
   {{-0.5f,0.5f},{1.0f,1.0f,0.0f}}
};

void checkValidationLayerSupport ()
{
   uint32_t layerCount;
   vkEnumerateInstanceLayerProperties (&layerCount, nullptr);
   std::vector<VkLayerProperties> availableLayers (layerCount);
   vkEnumerateInstanceLayerProperties (&layerCount, availableLayers.data ());

   std::vector<std::string> MissingValidationLayers;
   for (const auto& validationLayer : validationLayers)
   {
      auto matchExtension = [&] (const VkLayerProperties& layerProp) { return strcmp (layerProp.layerName, validationLayer) == 0; };

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

void PrintSupportedValidationLayers ()
{
   uint32_t layerCount;
   vkEnumerateInstanceLayerProperties (&layerCount, nullptr);
   std::vector<VkLayerProperties> availableLayers (layerCount);
   vkEnumerateInstanceLayerProperties (&layerCount, availableLayers.data ());

   std::cout << "available validation layers: " << std::endl;

   for (const auto& layer : availableLayers)
   {
      std::cout << "\t" << layer.layerName << std::endl;
   }
}

void checkInstanceExtensionSupport (std::vector<const char *> RequiredExtensions)
{
   uint32_t supportedExtensionCount = 0;
   vkEnumerateInstanceExtensionProperties (nullptr, &supportedExtensionCount, nullptr);
   std::vector<VkExtensionProperties> SupportedExtensions (supportedExtensionCount);
   vkEnumerateInstanceExtensionProperties (nullptr, &supportedExtensionCount, SupportedExtensions.data ());

   
   std::vector<std::string> MissingExtensions;

   for (const auto& RequiredExtension : RequiredExtensions)
   {
      auto matchExtension = [&] (const VkExtensionProperties& extensionProp) { return strcmp (extensionProp.extensionName, RequiredExtension) == 0; };

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

void PrintSupportedInstanceExtensions ()
{
   uint32_t extensionCount = 0;
   vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
   std::vector<VkExtensionProperties> extensions (extensionCount);
   vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, extensions.data ());

   std::cout << "available extensions: " << std::endl;

   for (const auto& extension : extensions)
   {
      std::cout << "\t" << extension.extensionName << std::endl;
   }
}

std::vector<const char *> getRequiredExtensions ()
{
   std::vector<const char *> extensions;

   unsigned int glfwExtensionCount = 0;
   const char** glfwExtensions;

   glfwExtensions = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);

   extensions.insert (extensions.end (), glfwExtensions, glfwExtensions + glfwExtensionCount);

   if (enableValidationLayers)
   {
      extensions.push_back (VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
   }

   return extensions;
}

void PrintAvailablePhysicalDevices (const VkInstance instance)
{
   uint32_t deviceCount = 0;
   vkEnumeratePhysicalDevices (instance, &deviceCount, nullptr);
   std::vector<VkPhysicalDevice> devices (deviceCount);
   vkEnumeratePhysicalDevices (instance, &deviceCount, devices.data ());

   std::cout << "available physical devices: " << std::endl;

   for (const auto& device : devices)
   {
      VkPhysicalDeviceProperties deviceProperties;
      vkGetPhysicalDeviceProperties (device, &deviceProperties);
      
      std::cout << "\t" << deviceProperties.deviceName << std::endl;
   }
}

VkResult CreateDebugReportCallbackEXT (VkInstance instance, const VkDebugReportCallbackCreateInfoEXT * pCreateInfo, const VkAllocationCallbacks * pAllocator, VkDebugReportCallbackEXT * pCallback)
{
   auto func = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr (instance, "vkCreateDebugReportCallbackEXT");
   if (func != nullptr)
   {
      return func (instance, pCreateInfo, pAllocator, pCallback);
   }
   else
   {
      return VK_ERROR_EXTENSION_NOT_PRESENT;
   }
}

void DestroyDebugReportCallbackEXT (
   VkInstance instance,
   VkDebugReportCallbackEXT callback,
   const VkAllocationCallbacks * pAllocator)
{
   auto func = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr (instance, "vkDestroyDebugReportCallbackEXT");
   if (func != nullptr)
   {
      func (instance, callback, pAllocator);
   }
}


bool checkDeviceExtensionSupport (VkPhysicalDevice device)
{
   uint32_t extensionCount;

   vkEnumerateDeviceExtensionProperties (device, nullptr, &extensionCount, nullptr);
   std::vector<VkExtensionProperties> availableExtensions (extensionCount);
   vkEnumerateDeviceExtensionProperties (device, nullptr, &extensionCount, availableExtensions.data ());

   std::set<std::string> requiredExtensions (deviceExtensions.begin (), deviceExtensions.end ());

   for (const auto& extension : availableExtensions)
   {
      requiredExtensions.erase (extension.extensionName);
   }

   return requiredExtensions.empty ();
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

HelloTriangleApplication::HelloTriangleApplication () : 
   physicalDevice(VK_NULL_HANDLE)
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

   HelloTriangleApplication* app = reinterpret_cast<HelloTriangleApplication*> (glfwGetWindowUserPointer (window));

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
   createGraphicsPipeline ();
   createFramebuffers ();
   createCommandPool ();
   createVertexBuffer ();
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


   VkApplicationInfo appInfo = {};
   appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
   appInfo.pApplicationName = "Hello Triangle";
   appInfo.applicationVersion = VK_MAKE_VERSION (1, 0, 0);
   appInfo.pEngineName = "No Engine";
   appInfo.engineVersion = VK_MAKE_VERSION (1, 0, 0);
   appInfo.apiVersion = VK_API_VERSION_1_0;

   VkInstanceCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
   createInfo.pApplicationInfo = &appInfo;

   PrintSupportedInstanceExtensions ();

   auto extensions = getRequiredExtensions ();

   checkInstanceExtensionSupport (extensions);
   
   createInfo.enabledExtensionCount = static_cast<uint32_t> (extensions.size ());
   createInfo.ppEnabledExtensionNames = extensions.data ();
   
   if (enableValidationLayers)
   {
      createInfo.enabledLayerCount = static_cast<uint32_t> (validationLayers.size ());
      createInfo.ppEnabledLayerNames = validationLayers.data ();
   }
   else
   {
      createInfo.enabledLayerCount = 0;
   }


   if (vkCreateInstance (&createInfo, nullptr, &instance) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create instance!");
   }

}

void HelloTriangleApplication::setupDebugCallback ()
{
   if (!enableValidationLayers) return;

   VkDebugReportCallbackCreateInfoEXT createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
   createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
   createInfo.pfnCallback = debugCallback;

   if (CreateDebugReportCallbackEXT (instance, &createInfo, nullptr, &callback) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to set up debug callback!");
   }
}

void HelloTriangleApplication::createSurface ()
{
   if (glfwCreateWindowSurface (instance, window, nullptr, &surface) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create window surface!");
   }
}

void HelloTriangleApplication::pickPhysicalDevice ()
{
   uint32_t deviceCount = 0;
   vkEnumeratePhysicalDevices (instance, &deviceCount, nullptr);

   if (deviceCount == 0)
   {
      throw std::runtime_error ("failed to find GPUs with Vulkan support!");
   }

   PrintAvailablePhysicalDevices (instance);

   std::vector<VkPhysicalDevice> devices (deviceCount);
   vkEnumeratePhysicalDevices (instance, &deviceCount, devices.data ());

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

bool HelloTriangleApplication::isDeviceSuitable (VkPhysicalDevice device)
{
   QueueFamilyIndices indices = findQueueFamilies (device);

   bool extensionsSupported = checkDeviceExtensionSupport (device);

   bool swapChainAdequate = false;

   if (extensionsSupported)
   {
      SwapChainSupportDetails swapChainSupport = querySwapChainSupport (device);
      swapChainAdequate = !swapChainSupport.formats.empty () && !swapChainSupport.presentModes.empty ();

   }

   return indices.isComplete () && extensionsSupported && swapChainAdequate;
}

inline QueueFamilyIndices HelloTriangleApplication::findQueueFamilies (VkPhysicalDevice device)
{
   QueueFamilyIndices indices;

   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties (device, &queueFamilyCount, nullptr);
   std::vector<VkQueueFamilyProperties> queueFamilies (queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties (device, &queueFamilyCount, queueFamilies.data ());

   int i = 0;
   for (const auto& queueFamily : queueFamilies)
   {
      if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
         indices.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      vkGetPhysicalDeviceSurfaceSupportKHR (device, i, surface, &presentSupport);

      if (queueFamily.queueCount > 0 && presentSupport)
      {
         indices.presentFamily = i;
      }


      if (indices.isComplete ())
      {
         break;
      }

      ++i;
   }

   return indices;
}

SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport (VkPhysicalDevice device)
{
   SwapChainSupportDetails details;

   vkGetPhysicalDeviceSurfaceCapabilitiesKHR (device, surface, &details.capabilities);

   uint32_t formatCount;
   vkGetPhysicalDeviceSurfaceFormatsKHR (device, surface, &formatCount, nullptr);

   if (formatCount != 0)
   {
      details.formats.resize (formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR (device, surface, &formatCount, details.formats.data ());
   }

   uint32_t presentCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR (device, surface, &presentCount, nullptr);

   if (presentCount != 0)
   {
      details.presentModes.resize (presentCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR (device, surface, &presentCount, details.presentModes.data ());
   }

   return details;
}

void HelloTriangleApplication::createSwapChain ()
{
   SwapChainSupportDetails swapChainSupport = querySwapChainSupport (physicalDevice);

   VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat (swapChainSupport.formats);
   VkPresentModeKHR presentMode = chooseSwapPresentMode (swapChainSupport.presentModes);

   VkExtent2D extent = chooseSwapExtent (swapChainSupport.capabilities);

   uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
   if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
   {
      imageCount = swapChainSupport.capabilities.maxImageCount;
   }

   VkSwapchainCreateInfoKHR createInfo = {};

   createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   createInfo.surface = surface;

   createInfo.minImageCount = imageCount;
   createInfo.imageFormat = surfaceFormat.format;
   createInfo.imageColorSpace = surfaceFormat.colorSpace;
   createInfo.imageExtent = extent;
   createInfo.imageArrayLayers = 1; //Always 1 for Non-Stereoscopic
   createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   
   QueueFamilyIndices indices = findQueueFamilies (physicalDevice);
   uint32_t queueFamilyIndices[] = {(uint32_t) indices.graphicsFamily, (uint32_t) indices.presentFamily};
   
   if (indices.graphicsFamily != indices.presentFamily)
   {
      createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
      createInfo.queueFamilyIndexCount = 2;
      createInfo.pQueueFamilyIndices = queueFamilyIndices;
   }
   else
   {
      createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      createInfo.queueFamilyIndexCount = 0;
      createInfo.pQueueFamilyIndices = nullptr;
   }

   createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
   createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

   createInfo.presentMode = presentMode;
   createInfo.clipped = VK_TRUE;
   createInfo.oldSwapchain = VK_NULL_HANDLE;

   if (vkCreateSwapchainKHR (device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create swap chain!");
   }

   //Retrieve Image Handles
   vkGetSwapchainImagesKHR (device, swapChain, &imageCount, nullptr);
   swapChainImages.resize (imageCount);
   vkGetSwapchainImagesKHR (device, swapChain, &imageCount, swapChainImages.data ());

   swapChainImageFormat = surfaceFormat.format;
   swapChainExtent = extent;


}

VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
   if (availableFormats.size () == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
   {
      return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
   }

   for (const auto& availableFormat : availableFormats)
   {
      if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      {
         return availableFormat;
      }
   }

   //Could rank the available formats and choose the best but instead just take the first one
   return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode (const std::vector<VkPresentModeKHR>& availablePresentModes)
{

   VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

   for (const auto& availablePresentMode : availablePresentModes)
   {
      if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
      {
         return availablePresentMode;
      }
      else if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR)
      {
         bestMode = availablePresentMode;
      }
   }

   return bestMode;
}

VkExtent2D HelloTriangleApplication::chooseSwapExtent (const VkSurfaceCapabilitiesKHR & capabilities)
{
   if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max ())
   {
      return capabilities.currentExtent;
   }
   else
   {
      int width, height;
      glfwGetWindowSize (window, &width, &height);

      VkExtent2D actualExtent = {width, height};
      
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
      VkImageViewCreateInfo createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image = swapChainImages[i];

      createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format = swapChainImageFormat;

      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;

      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0;
      createInfo.subresourceRange.levelCount = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount = 1;

      if (vkCreateImageView (device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
      {
         throw std::runtime_error ("failed to create image views!");
      }
   }
}

void HelloTriangleApplication::createRenderPass ()
{
   VkAttachmentDescription colorAttachment = {};
   colorAttachment.format = swapChainImageFormat;
   colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
   colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

   colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

   colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

   VkAttachmentReference colorAttachmentRef = {};
   colorAttachmentRef.attachment = 0;
   colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = 1;
   subpass.pColorAttachments = &colorAttachmentRef;
   
   VkSubpassDependency dependency = {};
   dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
   dependency.dstSubpass = 0;
   dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependency.srcAccessMask = 0;
   dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
   dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

   VkRenderPassCreateInfo renderPassInfo = {};
   renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.attachmentCount = 1;
   renderPassInfo.pAttachments = &colorAttachment;
   renderPassInfo.subpassCount = 1;
   renderPassInfo.pSubpasses = &subpass;
   renderPassInfo.dependencyCount = 1;
   renderPassInfo.pDependencies = &dependency;

   if (vkCreateRenderPass (device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
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

   VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
   VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);


   VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
   vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

   vertShaderStageInfo.module = vertShaderModule;
   vertShaderStageInfo.pName = "main";

   VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
   fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

   fragShaderStageInfo.module = fragShaderModule;
   fragShaderStageInfo.pName = "main";

   VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

   VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   
   auto bindingDescription = Vertex::getBindingDescription ();
   auto attributeDescription = Vertex::getAttributeDescriptions ();

   vertexInputInfo.vertexBindingDescriptionCount = 1;
   vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
   vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t> (attributeDescription.size ());
   vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data ();

   VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
   inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
   inputAssembly.primitiveRestartEnable = VK_FALSE;

   VkViewport viewport = {};
   viewport.x = 0.0f;
   viewport.y = 0.0f;

   viewport.width = (float) swapChainExtent.width;
   viewport.height = (float) swapChainExtent.height;
   viewport.minDepth = 0.0f;
   viewport.maxDepth = 0.0f;

   VkRect2D scissor = {};
   scissor.offset = {0, 0};
   scissor.extent = swapChainExtent;

   VkPipelineViewportStateCreateInfo viewportState = {};
   viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewportState.viewportCount = 1;
   viewportState.pViewports = &viewport;
   viewportState.scissorCount = 1;
   viewportState.pScissors = &scissor;

   VkPipelineRasterizationStateCreateInfo rasterizer = {};
   rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   rasterizer.depthClampEnable = VK_FALSE;
   rasterizer.rasterizerDiscardEnable = VK_FALSE;
   rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
   rasterizer.lineWidth = 1.0f;
   rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
   rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
   rasterizer.depthBiasEnable = VK_FALSE;
   rasterizer.depthBiasConstantFactor = 0.0f;
   rasterizer.depthBiasClamp = 0.0f;
   rasterizer.depthBiasSlopeFactor = 0.0f;

   VkPipelineMultisampleStateCreateInfo multisampling = {};
   multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampling.sampleShadingEnable = VK_FALSE;
   multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
   multisampling.minSampleShading = 1.0f;
   multisampling.pSampleMask = nullptr;
   multisampling.alphaToCoverageEnable = VK_FALSE;
   multisampling.alphaToOneEnable = VK_FALSE;

   VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
   colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   colorBlendAttachment.blendEnable = VK_FALSE;
   colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
   colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
   colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
   colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
   colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
   colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
   
   VkPipelineColorBlendStateCreateInfo colorBlending = {};
   colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   colorBlending.logicOpEnable = VK_FALSE;
   colorBlending.logicOp = VK_LOGIC_OP_COPY;
   colorBlending.attachmentCount = 1;
   colorBlending.pAttachments = &colorBlendAttachment;
   colorBlending.blendConstants[0] = 0.0f;
   colorBlending.blendConstants[1] = 0.0f;
   colorBlending.blendConstants[2] = 0.0f;
   colorBlending.blendConstants[3] = 0.0f;

   VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};
   VkPipelineDynamicStateCreateInfo dynamicState = {};
   dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   dynamicState.dynamicStateCount = 2;
   dynamicState.pDynamicStates = dynamicStates;

   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
   pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount = 0;
   pipelineLayoutInfo.pSetLayouts = nullptr;
   pipelineLayoutInfo.pushConstantRangeCount = 0;
   pipelineLayoutInfo.pPushConstantRanges = nullptr;

   if (vkCreatePipelineLayout (device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create the pipeline layout!");
   }

   VkGraphicsPipelineCreateInfo pipelineInfo = {};
   pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount = 2;
   pipelineInfo.pStages = shaderStages;
   pipelineInfo.pVertexInputState = &vertexInputInfo;
   pipelineInfo.pInputAssemblyState = &inputAssembly;
   pipelineInfo.pViewportState = &viewportState;
   pipelineInfo.pRasterizationState = &rasterizer;
   pipelineInfo.pMultisampleState = &multisampling;
   pipelineInfo.pDepthStencilState = nullptr;
   pipelineInfo.pColorBlendState = &colorBlending;
   pipelineInfo.pDynamicState = nullptr;

   pipelineInfo.layout = pipelineLayout;
   pipelineInfo.renderPass = renderPass;
   pipelineInfo.subpass = 0;

   if (vkCreateGraphicsPipelines (device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create graphics pipeline!");
   }

   vkDestroyShaderModule (device, fragShaderModule, nullptr);
   vkDestroyShaderModule (device, vertShaderModule, nullptr);
}

VkShaderModule HelloTriangleApplication::createShaderModule (const std::vector<char>& code)
{
   VkShaderModuleCreateInfo createInfo = {};

   createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize = code.size ();
   createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data ());

   VkShaderModule shaderModule;
   if (vkCreateShaderModule (device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
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
      VkImageView attachments[] = {swapChainImageViews[i]};

      VkFramebufferCreateInfo framebufferInfo = {};
      framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
      framebufferInfo.renderPass = renderPass;
      framebufferInfo.attachmentCount = 1;
      framebufferInfo.pAttachments = attachments;
      framebufferInfo.width = swapChainExtent.width;
      framebufferInfo.height = swapChainExtent.height;
      framebufferInfo.layers = 1;

      if (vkCreateFramebuffer (device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
      {
         throw std::runtime_error ("failed to create framebuffer!");
      }
   }
}

void HelloTriangleApplication::createCommandPool ()
{
   QueueFamilyIndices queueFamilyIndices = findQueueFamilies (physicalDevice);

   VkCommandPoolCreateInfo poolInfo = {};
   poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
   poolInfo.flags = 0;

   if (vkCreateCommandPool (device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create command pool!");
   }
}

void HelloTriangleApplication::createCommandBuffers ()
{
   commandBuffers.resize (swapChainFramebuffers.size ());

   VkCommandBufferAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool = commandPool;
   allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   allocInfo.commandBufferCount = (uint32_t) commandBuffers.size ();

   if (vkAllocateCommandBuffers (device, &allocInfo, commandBuffers.data ()) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to allocate command buffers!");
   }

   for (size_t i = 0; i < commandBuffers.size (); ++i)
   {
      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
      beginInfo.pInheritanceInfo = nullptr;

      vkBeginCommandBuffer (commandBuffers[i], &beginInfo);

      VkRenderPassBeginInfo renderPassInfo = {};
      renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      renderPassInfo.renderPass = renderPass;
      renderPassInfo.framebuffer = swapChainFramebuffers[i];

      renderPassInfo.renderArea.offset = {0, 0};
      renderPassInfo.renderArea.extent = swapChainExtent;

      VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
      renderPassInfo.clearValueCount = 1;
      renderPassInfo.pClearValues = &clearColor;

      vkCmdBeginRenderPass (commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

      vkCmdBindPipeline (commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

      VkBuffer vertexBuffers[] = {vertexBuffer};
      VkDeviceSize offsets[] = {0};
      vkCmdBindVertexBuffers (commandBuffers[i], 0, 1, vertexBuffers, offsets);

      vkCmdDraw (commandBuffers[i], static_cast<uint32_t> (vertices.size ()), 1, 0, 0);

      vkCmdEndRenderPass (commandBuffers[i]);

      if (vkEndCommandBuffer (commandBuffers[i]) != VK_SUCCESS)
      {
         throw std::runtime_error ("failed to record command buffer!");
      }
   }

}

void HelloTriangleApplication::createSemaphores ()
{
   VkSemaphoreCreateInfo semaphoreInfo = {};
   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

   if (vkCreateSemaphore (device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS
       || vkCreateSemaphore (device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS
       )
   {
      throw std::runtime_error ("failed to create semaphores!");
   }
}


void HelloTriangleApplication::createLogicalDevice ()
{
   QueueFamilyIndices indices = findQueueFamilies (physicalDevice);

   std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   std::set<int> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

   float queuePriority = 1.0f;

   for (int queueFamily : uniqueQueueFamilies)
   {
      VkDeviceQueueCreateInfo queueCreateInfo = {};
      queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back (queueCreateInfo);
   }

   VkPhysicalDeviceFeatures deviceFeatures = {};
   
   VkDeviceCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

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

   if (vkCreateDevice (physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create logical device!");
   }

   vkGetDeviceQueue (device, indices.graphicsFamily, 0, &graphicsQueue);
   vkGetDeviceQueue (device, indices.presentFamily, 0, &presentQueue);
}

void HelloTriangleApplication::mainLoop ()
{
   while (!glfwWindowShouldClose (window))
   {
      glfwPollEvents ();
      drawFrame ();
   }

   vkDeviceWaitIdle (device);
}

void HelloTriangleApplication::drawFrame ()
{
   uint32_t imageIndex;
   VkResult result = vkAcquireNextImageKHR (device, swapChain, std::numeric_limits<uint64_t>::max (), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

   if (result == VK_ERROR_OUT_OF_DATE_KHR)
   {
      recreateSwapChain ();
      return;
   }
   else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
   {
      throw std::runtime_error ("failed to acquire swap chain image!");
   }

   VkSubmitInfo submitInfo = {};
   submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
   
   VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
   VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

   submitInfo.waitSemaphoreCount = 1;
   submitInfo.pWaitSemaphores = waitSemaphores;
   submitInfo.pWaitDstStageMask = waitStages;

   submitInfo.commandBufferCount = 1;
   submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

   VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pSignalSemaphores = signalSemaphores;

   if (vkQueueSubmit (graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to submit draw command buffer!");
   }

   VkPresentInfoKHR presentInfo = {};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pWaitSemaphores = signalSemaphores;

   VkSwapchainKHR swapChains[] = {swapChain};
   presentInfo.swapchainCount = 1;
   presentInfo.pSwapchains = swapChains;
   presentInfo.pImageIndices = &imageIndex;
   presentInfo.pResults = nullptr;


   result = vkQueuePresentKHR (presentQueue, &presentInfo);

   if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
   {
      recreateSwapChain ();
   }
   else if (result != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to present swap chain image!");
   }

   vkQueueWaitIdle (presentQueue);
}

void HelloTriangleApplication::recreateSwapChain ()
{
   vkDeviceWaitIdle (device);

   cleanupSwapChain ();

   createSwapChain ();
   createImageViews ();
   createRenderPass ();
   createGraphicsPipeline ();
   createFramebuffers ();
   createCommandBuffers ();
}

void HelloTriangleApplication::cleanupSwapChain ()
{
   for (size_t i = 0; i < swapChainFramebuffers.size (); ++i)
   {
      vkDestroyFramebuffer (device, swapChainFramebuffers[i], nullptr);
   }

   vkFreeCommandBuffers (device, commandPool, static_cast<uint32_t> (commandBuffers.size ()), commandBuffers.data ());

   vkDestroyPipeline (device, graphicsPipeline, nullptr);

   vkDestroyPipelineLayout (device, pipelineLayout, nullptr);

   vkDestroyRenderPass (device, renderPass, nullptr);

   for (size_t i = 0; i < swapChainImageViews.size (); ++i)
   {
      vkDestroyImageView (device, swapChainImageViews[i], nullptr);
   }

   vkDestroySwapchainKHR (device, swapChain, nullptr);
}

void HelloTriangleApplication::createVertexBuffer ()
{
   VkBufferCreateInfo bufferInfo = {};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;

   bufferInfo.size = sizeof (vertices[0]) * vertices.size ();
   bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   if (vkCreateBuffer (device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create vertex buffer!");
   }

   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements (device, vertexBuffer, &memRequirements);

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize = memRequirements.size;
   allocInfo.memoryTypeIndex = findMemoryType (memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

   if (vkAllocateMemory (device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to allocate vertex buffer memory!");
   }

   vkBindBufferMemory (device, vertexBuffer, vertexBufferMemory, 0);

   void* data;
   vkMapMemory (device, vertexBufferMemory, 0, bufferInfo.size, 0, &data);
   memcpy (data, vertices.data (), (size_t) bufferInfo.size);
   vkUnmapMemory (device, vertexBufferMemory);

}

uint32_t HelloTriangleApplication::findMemoryType (uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
   VkPhysicalDeviceMemoryProperties memProperties;
   vkGetPhysicalDeviceMemoryProperties (physicalDevice, &memProperties);

   for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
   {
      if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties))
      {
         return i;
      }
   }

   throw std::runtime_error ("failed to find suitable memory type!");
}

void HelloTriangleApplication::cleanup ()
{
   cleanupSwapChain ();

   vkDestroyBuffer (device, vertexBuffer, nullptr);
   vkFreeMemory (device, vertexBufferMemory, nullptr);

   vkDestroySemaphore (device, renderFinishedSemaphore, nullptr);
   vkDestroySemaphore (device, imageAvailableSemaphore, nullptr);

   vkDestroyCommandPool (device, commandPool, nullptr);

   vkDestroyDevice (device, nullptr);

   DestroyDebugReportCallbackEXT (instance, callback, nullptr);

   vkDestroySurfaceKHR (instance, surface, nullptr);
   vkDestroyInstance (instance, nullptr);

   glfwDestroyWindow (window);

   glfwTerminate ();
}

VkVertexInputBindingDescription Vertex::getBindingDescription ()
{
   VkVertexInputBindingDescription bindingDescription = {};

   bindingDescription.binding = 0;
   bindingDescription.stride = sizeof (Vertex);
   bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

   return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 2> Vertex::getAttributeDescriptions ()
{
   std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

   attributeDescriptions[0].binding = 0;
   attributeDescriptions[0].location = 0;
   attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
   attributeDescriptions[0].offset = offsetof (Vertex, pos);

   attributeDescriptions[1].binding = 0;
   attributeDescriptions[1].location = 1;
   attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescriptions[1].offset = offsetof (Vertex, color);

   return attributeDescriptions;
}
