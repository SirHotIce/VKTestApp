//
// Created by Vin on 6/8/2025.
//

#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Extras {

    struct QueueFamilyIndices {
    public:
         int graphicsFamily=-1;
         int presentationFamily=-1;
         bool allQuesPresent(){
            return graphicsFamily>=0 && presentationFamily>=0;
        }
    };

    //the list of extensions we want our LD to support and check for
    const static std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;          //Surface properties, ie size, extent etc
        std::vector<VkSurfaceFormatKHR> formats;        //Surface image formats, eg: RGBA, RGB etc and size of each color
        std::vector<VkPresentModeKHR> presentModes;     //the presentation mode for the swap chaim, mailbox, instant, fifo or fifo relaxed
    };

    struct SwapchainImage {
        VkImage image;
        VkImageView imageView;
    };
} // Extras

#endif //UTILS_H
