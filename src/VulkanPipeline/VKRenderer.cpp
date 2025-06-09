//
// Created by Vinayak Regmi on 08/06/2025.
//

#include "VKRenderer.h"

#include <exception>
#include <iostream>
#include <ostream>
#include <set>
#include <stdexcept>
#include <glm/fwd.hpp>


void VKRenderer::createInstance() {
    //app info is mostly for storing info for refrence, only imp thing it stores is the version of vulkan that will be used
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    std::vector<const char*> extensions = std::vector<const char*>();
    extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    uint32_t extensionCount = 0;//glfw extension count for how many ext there are in glfw for this env
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&extensionCount);//get the required ext and store how many there are

    for (uint32_t i = 0; i < extensionCount; i++) {
        extensions.push_back(glfwExtensions[i]);
    }


    if(!checkExtensionSupport(extensions)) {
        throw std::runtime_error("Couldn't Initialize Vulkan Instance as some instances couldn't be found.");
    }

    createInfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

    createInfo.enabledExtensionCount = (uint32_t)(extensions.size());//explicit cast to exact type to prevent possible data loss with implicit cast
    createInfo.ppEnabledExtensionNames = extensions.data();

    //TODO: Set Validation Layer Count and Names
    createInfo.enabledLayerCount=0;
    createInfo.ppEnabledLayerNames = nullptr;



    VkResult vkInstanceStatus = vkCreateInstance(&createInfo, nullptr, &instance);//the central variable is for mem alloc i dont do that shi, Vulkan is pretty decent w/ that by def.
    if (vkInstanceStatus != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }else {
        std::cout << "Vulkan instance created." << std::endl;
    }

}

void VKRenderer::createSurface() {

    //internally glfw is  creating a create surface struct and populating all its values based on the platform
    VkResult surfaceCreationResult= glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if (surfaceCreationResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }
}

bool VKRenderer::checkExtensionSupport(std::vector<const char *> checkExtensions) {
    // Need to get number of extensions to create array of correct size to hold extensio
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    // Create a list of VkExtensionProperties using count
    std::vector<VkExtensionProperties> extensions;
    extensions.resize(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    // check if given extensions are in list of available extensions
    for (const auto &checkExtension: checkExtensions) {
        bool hasExtension = false;
        for (const auto &extension : extensions) {
            if (strcmp(checkExtension, extension. extensionName) != 0) {
                hasExtension = true;
                break;
            }
        }
        if (!hasExtension) {
            return false;
        }
    }
    std::cout << "Vulkan extensions all found." << std::endl;
    return true;
}

void VKRenderer::getPhysicalDevice() {
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    if (physicalDevices.size() == 0) {
        throw std::runtime_error("No Vulkan physical devices found.");
    }
    // //temp
    // devices.physicalDevice = physicalDevices[0];

    selectSupportedDevice(physicalDevices);
    if (devices.physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("Vulkan physical device is not supported.");
    }
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(devices.physicalDevice, &deviceProperties);
    std::cout << "Vulkan physical device: " << deviceProperties.deviceName << std::endl;

}

void VKRenderer::selectSupportedDevice(std::vector<VkPhysicalDevice> pDevices) {
    size_t noOfDevices = pDevices.size();
    for (size_t i = 0; i < noOfDevices; i++) {
        if (pDevices[i] != VK_NULL_HANDLE) {
            Extras::QueueFamilyIndices indices= checkIfQueueFamiliesAvailable(pDevices[i]);
            Extras::SwapChainSupportDetails swapChainSupport= querySwapChainSupport(pDevices[i]);
            //the device is only valid if, both presnet and graphic que are ther, it supports the swapchain deevice ext,and it returns proper swapchain formmat and present modes
            if (indices.allQuesPresent() && checkDeviceExtensionSupport(pDevices[i]) && !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty()) {
                devices.physicalDevice = pDevices[i];
                devices.pDsQueFamilies= indices;
                devices.swapChainSupport= swapChainSupport;
                return;

            }
        }
    }
    devices.physicalDevice = VK_NULL_HANDLE;

}

void VKRenderer::createLogicalDevice() {


    //the ques the ld will use are to be created using this info
    //never create same index q famm twice, itll crash
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    //set can only hold unique stuff, so if these two are same only 1 will stick around
    std::set<int> qFamIndices={devices.pDsQueFamilies.graphicsFamily, devices.pDsQueFamilies.presentationFamily};

    for (auto queue_index: qFamIndices) {
        VkDeviceQueueCreateInfo graphicQueueCreateInfo = {};
        graphicQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        graphicQueueCreateInfo.queueFamilyIndex = queue_index;
        graphicQueueCreateInfo.queueCount = 1; //how mmany ques to create
        float priority = 1.0f;
        graphicQueueCreateInfo.pQueuePriorities = &priority; //Vulkan  needs to know how to handle multiple ques so we assign priority, 1= highest priority, 0 is lower

        queueCreateInfos.push_back(graphicQueueCreateInfo);

    }

    //info to create the ld itself
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(Extras::deviceExtensions.size()); //no of enabled logical dev ext
    createInfo.ppEnabledExtensionNames = Extras::deviceExtensions.data(); //list of enabled LD extensions


    //features the device will be using, there area alot of bools in this struct, we can enable each feature, by def they are false
    VkPhysicalDeviceFeatures deviceFeatures = {};
    createInfo.pEnabledFeatures = &deviceFeatures; // all the features of the PD that the LD will use, list is defined above

    //create the ld
    VkResult lDcreateResult= vkCreateDevice(devices.physicalDevice, &createInfo, nullptr, &devices.logicalDevice);

    if (lDcreateResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create logical device");
    }
    //Ques are created at the time the LD is created and here wea re just accessing the g Q from the created LD and storing a refrence to it
    vkGetDeviceQueue(devices.logicalDevice, devices.pDsQueFamilies.graphicsFamily, 0, &graphicsQueue);
    //here:: from given LD, of given Q fam, of given index (0 since only  one q), where to store
    vkGetDeviceQueue(devices.logicalDevice, devices.pDsQueFamilies.presentationFamily, 0, &presentationQueue);
    //gettinmg infop into the handle


}

Extras::QueueFamilyIndices VKRenderer::checkIfQueueFamiliesAvailable(VkPhysicalDevice pd) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(pd, &queueFamilyCount, queueFamilies.data());

    int index = 0;
    Extras::QueueFamilyIndices indices;
    for (const auto &queue_family: queueFamilies) {
        if (queue_family.queueCount>0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = index;
        }
        //can also check others if they are there
        VkBool32 presentSupport = false;
        //here we are checking if this pd supports presentation que, in the crrent index, basically current q fam index and passing our surface which has been made by glfw so it checks for particular support
        vkGetPhysicalDeviceSurfaceSupportKHR(pd,index, surface, &presentSupport);
        if (queue_family.queueCount>0 && presentSupport) {
            indices.presentationFamily = index;
        }
        //once both indices are found then it will return true
        if (indices.allQuesPresent()) {
            break;
        }
        index++;
    }

    return indices;
}

//.this will give us what sort of capabilities, formmats and presentation modes does our gpu support for the swap chains
Extras::SwapChainSupportDetails VKRenderer::querySwapChainSupport(VkPhysicalDevice device) {
    Extras::SwapChainSupportDetails swapChainSupport;

    //gets the surface capabilities for the given surface on the given physical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainSupport.capabilities);

    //fetch format
    uint32_t formatCount=0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    //if the formats are returned as more than zero get the list of formats
    if (formatCount!=0) {

        //in vulkan we always resize any vector before use, as we usually pass them as refrence to a fn and the fn if the size is not exact will not set it, if less we miss data, so it is immportant to actually resizze all the lists before use in vulkan
        swapChainSupport.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, swapChainSupport.formats.data());
    }


    //same as with formats but for presentation support

    uint32_t presentationCount=0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, nullptr);

    if (presentationCount!=0) {
        swapChainSupport.presentModes.resize(presentationCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentationCount, swapChainSupport.presentModes.data());
    }

    return swapChainSupport;

}

bool VKRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    if (extensionCount==0) {
        return false;
    }

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
//compare the extensions we want from our list to the device supported exts and see if it is supported
    for (const auto &deviceExtension: Extras::deviceExtensions) {
        bool hasExtension = false;
        for (const auto &extension: availableExtensions) {
            if (strcmp(deviceExtension, extension.extensionName) == 0) {
                hasExtension = true;
                break;
            }
        }

        //since we looped all extensions in the device to check if it had the one we needed, and if it didnt then this device is unusable
        if (!hasExtension) {
            return false;
        }
    }
    return true;
}

VKRenderer::VKRenderer(GLFWwindow *window) {
    this->window = window;
}

int VKRenderer::initVulkan() {
    try {
        createInstance();
        createSurface();
        getPhysicalDevice();
        createLogicalDevice();
    }catch (std::runtime_error &e) {
        std::cerr << "What the hell, what the helly. Vulkan didnt Initialize.\n"<< e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void VKRenderer::cleanupVulkan() {
    if (instance!=VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        vkDestroyDevice(devices.logicalDevice, nullptr);

    }
}

VkInstance& VKRenderer::getInstance() {
    return instance;
}

VKRenderer::~VKRenderer() {
    //cleanupVulkan();
}
