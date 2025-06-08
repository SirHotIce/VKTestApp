//
// Created by Vinayak Regmi on 08/06/2025.
//
#ifndef  GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#endif
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#ifndef VKRENDERER_H
#define VKRENDERER_H


class VKRenderer {
private:
    VkInstance instance;
    GLFWwindow* window;
    void createInstance();
    bool checkExtensionSupport(std::vector<const char*> extensions);

public:
    VKRenderer(GLFWwindow* window);
    int initVulkan();
    void cleanupVulkan();
    VkInstance& getInstance();
    ~VKRenderer();
};



#endif //VKRENDERER_H
