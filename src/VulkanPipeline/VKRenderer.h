//
// Created by Vinayak Regmi on 08/06/2025.
//
#define  GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan.h>
#include "VKFrame.h"
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
    struct BestSwapChainSetup {
        VkSurfaceFormatKHR surfaceFormat;
        VkPresentModeKHR presentMode;
        VkExtent2D extent;

    } bestSwapchainSettings;
    VkQueue graphicsQueue, presentationQueue;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    std::vector<Extras::SwapchainImage> swapChainImages;             //making a list to store the images that are in the swap chain in our custom struct that holds immage and image view, image is like a physical so we are not creatinf but accessing it, but to interface with it we need image view
    VKFrame* vkFrame;
    GLFWwindow* window;
    void createInstance();
    bool checkExtensionSupport(std::vector<const char*> extensions);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    void createSurface();
    void createSwapChain();
    void getPhysicalDevice();
    void selectSupportedDevice(std::vector<VkPhysicalDevice> pDevices);
    void createLogicalDevice();
    void createPipeline();
    std::vector<char> readShaderFile(const std::string& fileName);
    VkShaderModule createShaderModule(std::vector<char> code);
    VkImageView createImageViewFromImage(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats);
    VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> presentModes);
    VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities);
    Extras::SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    Extras::QueueFamilyIndices checkIfQueueFamiliesAvailable(VkPhysicalDevice pd);

public:
    VKRenderer(VKFrame* _frame);
    int initVulkan();
    void cleanupVulkan();
    VkInstance& getInstance();
    ~VKRenderer();
};



#endif //VKRENDERER_H
