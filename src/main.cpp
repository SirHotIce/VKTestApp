#include "VulkanPipeline/VKFrame.h"
#include <iostream>

#include "VulkanPipeline/VKRenderer.h"

int main() {
    VKFrame vk_frame(800, 600, "Vulkan Test App");
    VKRenderer renderer(vk_frame.getWindow());
    if(renderer.initVulkan()==EXIT_FAILURE) {
        std::cout << "Failed to initialize Vulkan" << std::endl;
        return EXIT_FAILURE;
    }
    while (!glfwWindowShouldClose(vk_frame.getWindow())) {
        glfwPollEvents();
    }
    vk_frame.closeInstance();
    renderer.cleanupVulkan();
    return EXIT_SUCCESS;
}
