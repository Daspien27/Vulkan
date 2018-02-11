#pragma once

#include <array>
#include <vector>
#include <set>

#define GLFW_INCLUDE_VULKAN //Includes <vulkan\vulkan.h> indicates that glfw is to load in Vulkan
#include <GLFW/glfw3.h>

#include <vulkan\vulkan.hpp>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


struct Vertex
{
   glm::vec3 pos;
   glm::vec3 color;
   glm::vec2 texCoord;

   static vk::VertexInputBindingDescription getBindingDescription ();
   static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions ();

   bool operator == (const Vertex& other) const
   {
      return pos == other.pos && color == other.color && texCoord == other.texCoord;
   }
};

namespace std
{
   template <> struct hash<Vertex>
   {
      size_t operator()(Vertex const& vertex) const
      {
         return ((hash<glm::vec3> ()(vertex.pos) ^ (hash<glm::vec3> ()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2> ()(vertex.texCoord) << 1);
      }
   };
}

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

   QueueFamilyIndices () : graphicsFamily (-1), presentFamily (-1), transferFamily (-1) {}

   bool isComplete ()
   {
      return graphicsFamily >= 0 && presentFamily >= 0 && transferFamily >= 0;
   }
};

struct SwapChainSupportDetails
{
   vk::SurfaceCapabilitiesKHR capabilities;
   std::vector<vk::SurfaceFormatKHR> formats;
   std::vector<vk::PresentModeKHR> presentModes;
};


class HelloTriangleApplication
{
private:

   GLFWwindow * window;
   vk::Instance instance;
   vk::DebugReportCallbackEXT callback;
   vk::DebugReportCallbackEXT debugReport;

   vk::SurfaceKHR surface;
   vk::PhysicalDevice physicalDevice;
   vk::Device device;
   vk::Queue graphicsQueue;
   vk::Queue presentQueue;
   vk::Queue transferQueue;

   vk::SwapchainKHR swapChain;
   std::vector<vk::Image> swapChainImages;
   vk::Format swapChainImageFormat;
   vk::Extent2D swapChainExtent;
   std::vector<vk::ImageView> swapChainImageViews;
   std::vector<vk::Framebuffer> swapChainFramebuffers;

   vk::RenderPass renderPass;
   vk::DescriptorSetLayout descriptorSetLayout;
   vk::PipelineLayout pipelineLayout;
   vk::Pipeline graphicsPipeline;

   std::vector<Vertex> vertices;
   std::vector<uint32_t> indices;
   vk::Buffer vertexBuffer;
   vk::DeviceMemory vertexBufferMemory;

   vk::Buffer indexBuffer;
   vk::DeviceMemory indexBufferMemory;

   vk::Buffer uniformBuffer;
   vk::DeviceMemory uniformBufferMemory;

   vk::CommandPool commandPoolGraphics;
   vk::CommandPool commandPoolTransfer;
   std::vector<vk::CommandBuffer> commandBuffers;

   vk::DescriptorPool descriptorPool;
   vk::DescriptorSet descriptorSet;

   vk::Semaphore imageAvailableSemaphore;
   vk::Semaphore renderFinishedSemaphore;

   vk::Image textureImage;
   vk::DeviceMemory textureImageMemory;
   vk::ImageView textureImageView;
   vk::Sampler textureSampler;

   vk::Image depthImage;
   vk::DeviceMemory depthImageMemory;
   vk::ImageView depthImageView;



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
   bool isDeviceSuitable (vk::PhysicalDevice device);
   QueueFamilyIndices findQueueFamilies (vk::PhysicalDevice device);

   void createLogicalDevice ();

   void createSwapChain ();
   SwapChainSupportDetails querySwapChainSupport (vk::PhysicalDevice device);
   vk::SurfaceFormatKHR chooseSwapSurfaceFormat (const std::vector<vk::SurfaceFormatKHR>& availableFormats);
   vk::PresentModeKHR chooseSwapPresentMode (const std::vector<vk::PresentModeKHR>& availablePresentModes);
   vk::Extent2D chooseSwapExtent (const vk::SurfaceCapabilitiesKHR& capabilities);

   void createImageViews ();

   vk::ImageView createImageView (vk::Image & image, vk::Format format, vk::ImageAspectFlags aspectFlags);

   void createRenderPass ();

   void createGraphicsPipeline ();
   vk::ShaderModule createShaderModule (const std::vector<char>& code);

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

   void createDescriptorPool ();
   void createDescriptorSet ();

   void createBuffer (vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
   void copyBuffer (vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

   void createDescriptorSetLayout ();

   void createTextureImage ();
   void createImage (uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::Image& image, vk::DeviceMemory& imageMemory);
   void transitionImageLayout (vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandPool commandPool, vk::Queue queue);
   void copyBufferToImage (vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

   void createTextureImageView ();
   void createTextureSampler ();

   void createDepthResources ();
   vk::Format findSupportedFormat (const std::vector<vk::Format>& candidates, vk::ImageTiling, vk::FormatFeatureFlags features);
   vk::Format findDepthFormat ();
   bool hasStencilComponent (vk::Format format);

   void loadModel ();

   uint32_t findMemoryType (uint32_t typeFilter, vk::MemoryPropertyFlags properties);

   vk::CommandBuffer beginSingleTimeCommands (vk::CommandPool& commandPool);
   void endSingleTimeCommands (vk::CommandBuffer commandBuffer, vk::CommandPool commandPool, vk::Queue queue);

   void cleanup ();
};