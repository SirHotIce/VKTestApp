//
// Created by Vinayak Regmi on 08/06/2025.
//

#include "VKRenderer.h"

#include <exception>
#include <fstream>
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

void VKRenderer::createSwapChain() {
    //select best format, p mode and extent
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(devices.swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(devices.swapChainSupport.presentModes);
    VkExtent2D swapChainExtent= chooseSwapExtent(devices.swapChainSupport.capabilities);
    //how many images are in the swap chain, we want 1 more than the min to allow for a triple buffer
    uint32_t imageCount= devices.swapChainSupport.capabilities.minImageCount + 1;
    if (imageCount > devices.swapChainSupport.capabilities.maxImageCount) {
        imageCount = devices.swapChainSupport.capabilities.maxImageCount;
    }
    //create info
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface= surface;
    createInfo.imageFormat= surfaceFormat.format;
    createInfo.imageColorSpace= surfaceFormat.colorSpace;
    createInfo.presentMode= presentMode;
    createInfo.imageExtent= swapChainExtent;
    createInfo.minImageCount= imageCount;
    createInfo.imageArrayLayers= 1;     //Number of layers for each immage in chain
    createInfo.imageUsage= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;  //how this swap chain will be used, usually we want it as image, for depth we can do it in FBO
    createInfo.preTransform= devices.swapChainSupport.capabilities.currentTransform; //transforms to perfrom on current swwap chain
    createInfo.compositeAlpha= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; //how to handle blending image with external graphics (eg: other windows)
    createInfo.clipped= VK_TRUE;        //whehter to clip the parts of images not in view or not

    //check if ques are independent and assign
    if (devices.pDsQueFamilies.graphicsFamily!=devices.pDsQueFamilies.presentationFamily) {
        uint32_t queFamilyIndices[]= {(uint32_t)devices.pDsQueFamilies.graphicsFamily, (uint32_t)devices.pDsQueFamilies.presentationFamily};
        createInfo.imageSharingMode= VK_SHARING_MODE_CONCURRENT;//both ques share this sw chain if the ques are diff
        createInfo.queueFamilyIndexCount= 2; //since there are 2
        createInfo.pQueueFamilyIndices=queFamilyIndices;

    }else {//if they are both same we wont be sharing and the active one will use it
        createInfo.imageSharingMode= VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount= 0;
        createInfo.pQueueFamilyIndices= nullptr;
    }

    createInfo.oldSwapchain= nullptr;

    VkResult res= vkCreateSwapchainKHR(devices.logicalDevice, &createInfo, nullptr, &swapChain);
    if (res!=VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan swapchain");
    }

    //save the vars
    bestSwapchainSettings.surfaceFormat=surfaceFormat;

    bestSwapchainSettings.presentMode= presentMode;

    bestSwapchainSettings.extent= swapChainExtent;

    //get the swap chain images
    uint32_t swpChainImmageCount;
    vkGetSwapchainImagesKHR(devices.logicalDevice, swapChain, &swpChainImmageCount, nullptr);
    std::vector<VkImage> images(swpChainImmageCount);
    vkGetSwapchainImagesKHR(devices.logicalDevice, swapChain, &swpChainImmageCount, images.data());

    //now we need to reitterate through each of these images and store them as our struct in the list that we have for images
    for (auto image: images) {
        //store the image and its view that is created in the fn into the list

        Extras::SwapchainImage toStore={};
        toStore.image= image;
        toStore.imageView= createImageViewFromImage(image, surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);
        swapChainImages.push_back(toStore);


    }
}

VkImageView VKRenderer::createImageViewFromImage(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {

    VkImageView imageView;
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image= image;                                  //image to create view for
    viewInfo.format = format;                               //formmat of image data
    viewInfo.viewType= VK_IMAGE_VIEW_TYPE_2D;               //type of immage, 1D, 2D, Cube etc
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;     //this allows remapping of rgba values however, but we are just using themm as they are by swizzzling identity
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    //subresources allow to view only a part of an image
    viewInfo.subresourceRange.aspectMask= aspectFlags;      //which aspect we want the immage to view (Eg: Color_bit for viewing color)
    viewInfo.subresourceRange.baseMipLevel= 0;              //we want to start the mmipmap level at 0
    viewInfo.subresourceRange.levelCount= 1;                //no of mipmap levels to view
    viewInfo.subresourceRange.baseArrayLayer= 0;            //start array level to viewfrom
    viewInfo.subresourceRange.layerCount= 1;                 //Number of array levels to view

    //create image and return
    VkResult createResult= vkCreateImageView(devices.logicalDevice, &viewInfo, nullptr, &imageView);
    if (createResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to create image view");
    }
    return imageView;


}

VkSurfaceFormatKHR VKRenderer::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> formats) {
    //this meanss it ssuports any so we want to create our own as we want and passs
    if (formats.size()==1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        VkSurfaceFormatKHR surfaceFormat;
        surfaceFormat.format = VK_FORMAT_R8G8B8A8_SRGB;
        surfaceFormat.colorSpace= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        return  surfaceFormat;
    }

    //check if our preferred format is present
    for (auto format: formats) {
        if ((format.format == VK_FORMAT_R8G8B8A8_SRGB || format.format == VK_FORMAT_B8G8R8A8_SRGB) && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    //if not return the first one in the list
    return formats[0];

}

VkPresentModeKHR VKRenderer::chooseSwapPresentMode(std::vector<VkPresentModeKHR> presentModes) {
    //check if mailbox is present
    for (auto presentMode: presentModes) {
        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }

    //if not we return fifo as that is the default
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VKRenderer::chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities) {
    //if the extent is at the numeric liit then the extent can vary otherwise it is the size of the window
    if (capabilities.currentExtent.width!=std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int bWidth, bHeight;
        bWidth= vkFrame->getBufferSize().x;
        bHeight= vkFrame->getBufferSize().y;
        VkExtent2D extent;
        extent.width=static_cast<uint32_t>(bWidth);
        extent.height=static_cast<uint32_t>(bHeight);


        //surface also needed to define max and min width and height for the boundary to clamp the values
        extent.width= std::max(capabilities.minImageExtent.width,std::min(capabilities.maxImageExtent.width,extent.width));
        extent.height= std::max(capabilities.minImageExtent.height,std::min(capabilities.maxImageExtent.height,extent.height));
        return extent;
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

void VKRenderer::createPipeline() {
    auto vertShaderCode= readShaderFile("shaders/vert.spv");
    auto fragShaderCode= readShaderFile("shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

    //shader stage creation informmation
    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo = {};
    vertShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageCreateInfo.module = vertShaderModule;
    vertShaderStageCreateInfo.pName = "vert"; //name of the entry fn in vs
    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo = {};
    fragShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    fragShaderStageCreateInfo.module = fragShaderModule;
    fragShaderStageCreateInfo.pName = "frag"; //name of entry fn in fs

    VkPipelineShaderStageCreateInfo stages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    //pipeline creation

    //Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr; //List of Vertex Binding Description
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;//List of AP Descriptions (data format and where to bind to)

    //Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;  //Primitive typr to assemble to similar to telling gl to draw triangles
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; //Allow overriding of "strip" topology to start new primitives

    //Viewport and sccissor
    //Creaate a viewport info struct

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width= (float)bestSwapchainSettings.extent.width;
    viewport.height = (float)bestSwapchainSettings.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    //create a scissor info struct
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = bestSwapchainSettings.extent;

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;//set the created viewport as the main viewport
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;//set made scissor as main

    //Dynamic states go here ----not using yet----
    //can in future enable dyn state viewport and dy  state scissor for auto resizzing VP while resizing the window

    //Rasterizer

    VkPipelineRasterizationStateCreateInfo rasterizerStateCreateInfo = {};
    rasterizerStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizerStateCreateInfo.depthClampEnable = VK_FALSE;//basically clips stuff after a certain distance, needs to be checked and enabled in device features
    rasterizerStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;//whether to discard data and skip rasterizer, not suitable for pipelines with a Frame buffer, use case mostly with compute a nd compute to gen colors
    rasterizerStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;//how to fill the polygons, fully fill, leave as wireframe or point cloud
    rasterizerStateCreateInfo.lineWidth = 1.0f; //how thick the line should be when drawn
    rasterizerStateCreateInfo.cullMode= VK_CULL_MODE_BACK_BIT; //cull the back faces
    rasterizerStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;//set the winding order to CW or CCW
    rasterizerStateCreateInfo.depthBiasEnable= VK_FALSE;//this adds a slight bias in the depth map, like we do in OGL to remove shadow acne but this VK does it itself



    //Multisampling
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
    multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleStateCreateInfo.rasterizationSamples= VK_SAMPLE_COUNT_1_BIT; //Number of sammples to use perfragment, if i were doing msaa 4x i would sammple each frag 4x times and avg it


    //Blend Attachement to instruct how to handle the blending of colors
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
    colorBlendAttachmentState.colorWriteMask= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; //which bits to apply the blending to just bitwise operation here we apply to all
    colorBlendAttachmentState.blendEnable= VK_TRUE;//enable color blend

    //blending color uses the blend equation
    //blend eqn: (new color alpha * new color) + ((1- new color alpha)* old color)

    colorBlendAttachmentState.srcColorBlendFactor= VK_BLEND_FACTOR_ONE;
    colorBlendAttachmentState.dstColorBlendFactor= VK_BLEND_FACTOR_ZERO;
    colorBlendAttachmentState.alphaBlendOp= VK_BLEND_OP_ADD;
    //Summmarizzed ( 1 * new alpha) + (0 * old alpha) = just new alpjha


    //Color Blending
    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
    colorBlendStateCreateInfo.sType= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;//whether to blend colors logically or mathematically
    colorBlendStateCreateInfo.attachmentCount = 1;
    colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState; //attach the blend instructions attachment that we made abpve to this blend state

    //Pipeline Layout (TODO: Apply Future Descriptor set layouts)
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.setLayoutCount = 0; //currently no descriptor set so ct is 0
    pipelineLayoutCreateInfo.pSetLayouts = nullptr; //currently no d sets
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0; //no p constants so ct 0
    pipelineLayoutCreateInfo.pPushConstantRanges= nullptr; // no p consts rn


    VkResult pipelineCreationStatus= vkCreatePipelineLayout(devices.logicalDevice, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);

    if (pipelineCreationStatus!=VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }
    //Destroy the shader modules
    vkDestroyShaderModule(devices.logicalDevice, vertShaderModule, nullptr);
    vkDestroyShaderModule(devices.logicalDevice, fragShaderModule, nullptr);
}


std::vector<char> VKRenderer::readShaderFile(const std::string &fileName) {
    std::ifstream file(fileName, std::ios::binary|std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open shader file " + fileName);
    }
    //Get current read position to resize the file buffer
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> shaderData(fileSize);

    //move the read position to the srart
    file.seekg(0);

    //Read the file data into the buffer
    file.read(shaderData.data(), fileSize);
    //close stream
    file.close();
    return shaderData;

}

VkShaderModule VKRenderer::createShaderModule(std::vector<char> code) {
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<uint32_t*>(code.data());//it is not converting this to uint32_t this is telling the cpu to interpert this char as an uint

    VkShaderModule shaderModule;
    VkResult result= vkCreateShaderModule(devices.logicalDevice, &createInfo, nullptr, &shaderModule);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module");
    }
    return shaderModule;
}

VKRenderer::VKRenderer(VKFrame* _frame) {
    this->vkFrame = _frame;
    this->window = _frame->getWindow();
}

int VKRenderer::initVulkan() {
    try {
        createInstance();
        createSurface();
        getPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
    }catch (std::runtime_error &e) {
        std::cerr << "What the hell, what the helly. Vulkan didnt Initialize.\n"<< e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

void VKRenderer::cleanupVulkan() {
    if (instance!=VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(devices.logicalDevice, pipelineLayout, nullptr);
        for (auto image: swapChainImages) {
            vkDestroyImageView(devices.logicalDevice, image.imageView, nullptr);
        }
        vkDestroySwapchainKHR(devices.logicalDevice, swapChain, nullptr);
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
