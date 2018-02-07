#pragma once

#include <array>
#include <vector>
#include <set>

#define GLFW_INCLUDE_VULKAN //Includes <vulkan\vulkan.h> indicates that glfw is to load in Vulkan
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Vertex
{
   glm::vec2 pos;
   glm::vec3 color;

   static VkVertexInputBindingDescription getBindingDescription();
   static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions ();

};

struct UniformBufferObject
{
   glm::mat4 model;
   glm::mat4 view;
   glm::mat4 proj;
};

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
   int transferFamily;

   QueueFamilyIndices () : graphicsFamily (-1), presentFamily(-1), transferFamily(-1) {}

   bool isComplete ()
   {
      return graphicsFamily >= 0 && presentFamily >= 0 && transferFamily >= 0;
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
   VkQueue transferQueue;

   VkSwapchainKHR swapChain;
   std::vector<VkImage> swapChainImages;
   VkFormat swapChainImageFormat;
   VkExtent2D swapChainExtent;
   std::vector<VkImageView> swapChainImageViews;
   std::vector<VkFramebuffer> swapChainFramebuffers;

   VkRenderPass renderPass;
   VkDescriptorSetLayout descriptorSetLayout;
   VkPipelineLayout pipelineLayout;
   VkPipeline graphicsPipeline;
   
   VkBuffer vertexBuffer;
   VkDeviceMemory vertexBufferMemory;
   VkBuffer indexBuffer;
   VkDeviceMemory indexBufferMemory;
   VkBuffer uniformBuffer;
   VkDeviceMemory uniformBufferMemory;

   VkCommandPool commandPoolGraphics;
   VkCommandPool commandPoolTransfer;
   std::vector<VkCommandBuffer> commandBuffers;



   VkSemaphore imageAvailableSemaphore;
   VkSemaphore renderFinishedSemaphore;

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

   static void onWindowResized (GLFWwindow* window, int width, int height);

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

   void createRenderPass ();

   void createGraphicsPipeline ();
   VkShaderModule createShaderModule (const std::vector<char>& code);

   void createFramebuffers ();

   void createCommandPool ();
   void createCommandBuffers ();

   void createSemaphores ();

   void mainLoop ();
   void updateUniformBuffer ();
   void drawFrame ();

   void recreateSwapChain ();
   void cleanupSwapChain ();

   void createVertexBuffer ();
   void createIndexBuffer ();
   void createUniformBuffer ();

   void createBuffer (VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
   void copyBuffer (VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

   void createDescriptorSetLayout ();

   uint32_t findMemoryType (uint32_t typeFilter, VkMemoryPropertyFlags properties);

   void cleanup ();
};