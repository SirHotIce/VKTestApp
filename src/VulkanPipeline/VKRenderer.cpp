//
// Created by Vinayak Regmi on 08/06/2025.
//

#include "VKRenderer.h"

#include <exception>
#include <iostream>
#include <ostream>
#include <stdexcept>

void VKRenderer::createInstance() {
    //app info is mostly for storing info for refrence, only imp thing it stores is the version of vulkan that will be used
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Vulkan App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

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
        throw std::runtime_error("Failed to create Vulkan instance"+ std::to_string(vkInstanceStatus));
    }else {
        std::cout << "Vulkan instance created." << std::endl;
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

VKRenderer::VKRenderer(GLFWwindow *window) {
    this->window = window;
}

int VKRenderer::initVulkan() {
    try {
        createInstance();
    }catch (std::runtime_error &e) {
        std::cerr << "What the hell, what the helly. Vulkan didnt Initialize.\n"<< e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void VKRenderer::cleanupVulkan() {
    vkDestroyInstance(instance, nullptr);
}

VkInstance& VKRenderer::getInstance() {
    return instance;
}

VKRenderer::~VKRenderer() {
    cleanupVulkan();
}
