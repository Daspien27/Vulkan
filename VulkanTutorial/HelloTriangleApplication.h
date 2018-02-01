#pragma once

#define GLFW_INCLUDE_VULKAN //Includes <vulkan\vulkan.h> indicates that glfw is to load in Vulkan
#include <GLFW/glfw3.h>


class HelloTriangleApplication
{
private:

   GLFWwindow* window;
   VkInstance instance;

public:
   HelloTriangleApplication ();
   ~HelloTriangleApplication ();

   void run ();
private:
   void initWindow ();

   void initVulkan ();
   void createInstance ();

   static void PrintSupportedExtensions ();

   static void EnsureRequiredExtensionsAreSupported (const char ** RequiredExtensions, int NumRequiredExtensions);

   void mainLoop ();

   void cleanup ();
};