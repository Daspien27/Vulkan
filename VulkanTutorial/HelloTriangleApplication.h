#pragma once

struct GLFWwindow;


class HelloTriangleApplication
{
private:

   GLFWwindow* window;

public:
   HelloTriangleApplication ();
   ~HelloTriangleApplication ();

   void run ();
private:
   void initWindow ();
   void initVulkan ();
   void mainLoop ();
   void cleanup ();
};