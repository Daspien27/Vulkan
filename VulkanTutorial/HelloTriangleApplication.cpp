#include "HelloTriangleApplication.h"

#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

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

   return indices.isComplete ();
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

   createInfo.enabledExtensionCount = 0;

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
   vkDestroyDevice (device, nullptr);

   DestroyDebugReportCallbackEXT (instance, callback, nullptr);

   vkDestroySurfaceKHR (instance, surface, nullptr);
   vkDestroyInstance (instance, nullptr);

   glfwDestroyWindow (window);

   glfwTerminate ();
}
