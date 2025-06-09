//
// Created by Vinayak Regmi on 08/06/2025.
//
#define  GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#ifndef VKRENDERER_H
#define VKRENDERER_H
#include "../Extras/Utils.h"



class VKRenderer {
private:
    struct Devices {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
        Extras::QueueFamilyIndices pDsQueFamilies;
        Extras::SwapChainSupportDetails swapChainSupport;
    } devices;
    VkQueue graphicsQueue, presentationQueue;
    VkInstance instance;
    VkSurfaceKHR surface;
    GLFWwindow* window;
    void createInstance();
    bool checkExtensionSupport(std::vector<const char*> extensions);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void createSurface();
    void getPhysicalDevice();
    void selectSupportedDevice(std::vector<VkPhysicalDevice> pDevices);
    void createLogicalDevice();
    Extras::SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    Extras::QueueFamilyIndices checkIfQueueFamiliesAvailable(VkPhysicalDevice pd);
public:
    VKRenderer(GLFWwindow* window);
    int initVulkan();
    void cleanupVulkan();
    VkInstance& getInstance();
    ~VKRenderer();
};



#endif //VKRENDERER_H
