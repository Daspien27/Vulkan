#pragma once

#define GLFW_INCLUDE_VULKAN //Includes <vulkan\vulkan.h> indicates that glfw is to load in Vulkan
#include <GLFW/glfw3.h>

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

   QueueFamilyIndices () : graphicsFamily (-1) {}

   bool isComplete ()
   {
      return graphicsFamily >= 0;
   }
};



class HelloTriangleApplication
{
private:

   GLFWwindow* window;
   VkInstance instance;
   VkDebugReportCallbackEXT callback;
   VkPhysicalDevice physicalDevice;

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
   
   void pickPhysicalDevice ();
   bool isDeviceSuitable (VkPhysicalDevice device);
   QueueFamilyIndices findQueueFamilies (VkPhysicalDevice device);
   void mainLoop ();

   void cleanup ();
};