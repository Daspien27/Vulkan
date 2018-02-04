#pragma once

#define GLFW_INCLUDE_VULKAN //Includes <vulkan\vulkan.h> indicates that glfw is to load in Vulkan
#include <GLFW/glfw3.h>

#include <vector>
#include <set>

VkResult CreateDebugReportCallbackEXT (
   VkInstance instance,
   const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
   const VkAllocationCallbacks* pAllocator,
   VkDebugReportCallbackEXT* pCallback);
void DestroyDebugReportCallbackEXT (
   VkInstance instance,
   VkDebugReportCallbackEXT callback,
   const VkAllocationCallbacks* pAllocator);

struct QueueFamilyIndices
{
   int graphicsFamily;
   int presentFamily;

   QueueFamilyIndices () : graphicsFamily (-1) {}

   bool isComplete ()
   {
      return graphicsFamily >= 0 && presentFamily >= 0;
   }
};

struct SwapChainSupportDetails
{
   VkSurfaceCapabilitiesKHR capabilities;
   std::vector<VkSurfaceFormatKHR> formats;
   std::vector<VkPresentModeKHR> presentModes;
};


class HelloTriangleApplication
{
private:

   GLFWwindow* window;
   VkInstance instance;
   VkDebugReportCallbackEXT callback;
   VkSurfaceKHR surface;
   VkPhysicalDevice physicalDevice;
   VkDevice device;
   VkQueue graphicsQueue;
   VkQueue presentQueue;

   VkSwapchainKHR swapChain;
   std::vector<VkImage> swapChainImages;
   VkFormat swapChainImageFormat;
   VkExtent2D swapChainExtent;
   std::vector<VkImageView> swapChainImageViews;

public:
   HelloTriangleApplication ();
   ~HelloTriangleApplication ();

   void run ();

   static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback (
      VkDebugReportFlagsEXT flags,
      VkDebugReportObjectTypeEXT objType,
      uint64_t obj,
      size_t location,
      int32_t code,
      const char* layerPrefix,
      const char* msg,
      void* userData);

private:
   void initWindow ();

   void initVulkan ();
   void createInstance ();
   
   void setupDebugCallback ();
   
   void createSurface ();

   void pickPhysicalDevice ();
   bool isDeviceSuitable (VkPhysicalDevice device);
   QueueFamilyIndices findQueueFamilies (VkPhysicalDevice device);
   
   void createLogicalDevice ();

   void createSwapChain ();
   SwapChainSupportDetails querySwapChainSupport (VkPhysicalDevice device);
   VkSurfaceFormatKHR chooseSwapSurfaceFormat (const std::vector<VkSurfaceFormatKHR>& availableFormats);
   VkPresentModeKHR chooseSwapPresentMode (const std::vector<VkPresentModeKHR>& availablePresentModes);
   VkExtent2D chooseSwapExtent (const VkSurfaceCapabilitiesKHR& capabilities);

   void createImageViews ();

   void createGraphicsPipeline ();

   void mainLoop ();

   void cleanup ();
};