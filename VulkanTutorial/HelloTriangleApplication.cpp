#include "HelloTriangleApplication.h"

#include <functional>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

const int WIDTH = 800;
const int HEIGHT = 600;


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
}

void HelloTriangleApplication::createInstance ()
{
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

   unsigned int glfwExtensionCount = 0;
   const char** glfwExtensions;

   PrintSupportedExtensions ();

   glfwExtensions = glfwGetRequiredInstanceExtensions (&glfwExtensionCount);

   EnsureRequiredExtensionsAreSupported (glfwExtensions, glfwExtensionCount);
   
   createInfo.enabledExtensionCount = glfwExtensionCount;
   createInfo.ppEnabledExtensionNames = glfwExtensions;
   
   createInfo.enabledLayerCount = 0;

   if (vkCreateInstance (&createInfo, nullptr, &instance) != VK_SUCCESS)
   {
      throw std::runtime_error ("failed to create instance!");
   }

}

void HelloTriangleApplication::PrintSupportedExtensions ()
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

void HelloTriangleApplication::EnsureRequiredExtensionsAreSupported (const char ** RequiredExtensions, int NumRequiredExtensions)
{
   uint32_t extensionCount = 0;
   vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, nullptr);
   std::vector<VkExtensionProperties> extensions (extensionCount);
   vkEnumerateInstanceExtensionProperties (nullptr, &extensionCount, extensions.data ());

   std::vector<std::string> MissingExtensions;
   for (unsigned int i = 0; i < NumRequiredExtensions; ++i)
   {
      auto& RequiredExtension = RequiredExtensions[i];
      auto matchExtension = [&] (const VkExtensionProperties& extensionProp) { return strcmp (extensionProp.extensionName, RequiredExtension) == 0; };

      auto findIfRequiredExtensionSupported = std::find_if (std::begin (extensions), std::end (extensions), matchExtension);

      if (findIfRequiredExtensionSupported == std::end (extensions))
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


void HelloTriangleApplication::mainLoop ()
{
   while (!glfwWindowShouldClose (window))
   {
      glfwPollEvents ();
   }
}

void HelloTriangleApplication::cleanup ()
{
   vkDestroyInstance (instance, nullptr);

   glfwDestroyWindow (window);

   glfwTerminate ();
}
