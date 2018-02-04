#include "HelloTriangleApplication.h"

#include <functional>
#include <iostream>
#include <string>
#include <algorithm>
#include <numeric>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
   const bool enableValidationLayers = false
#else
   const bool enableValidationLayers = true;
#endif

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
   glfwWindowHint (GLFW_RESIZABLE, GLFW_FALSE);

   window = glfwCreateWindow (WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
   
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
   createGraphicsPipeline ();
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
      VkExtent2D actualExtent = {WIDTH, HEIGHT};
      
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

void HelloTriangleApplication::createGraphicsPipeline ()
{

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
   }
}

void HelloTriangleApplication::cleanup ()
{
   for (size_t i = 0; i < swapChainImageViews.size (); ++i)
   {
      vkDestroyImageView (device, swapChainImageViews[i], nullptr);
   }

   vkDestroySwapchainKHR (device, swapChain, nullptr);

   vkDestroyDevice (device, nullptr);

   DestroyDebugReportCallbackEXT (instance, callback, nullptr);

   vkDestroySurfaceKHR (instance, surface, nullptr);
   vkDestroyInstance (instance, nullptr);

   glfwDestroyWindow (window);

   glfwTerminate ();
}
