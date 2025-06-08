//
// Created by Vinayak Regmi on 08/06/2025.
//

#include "VKFrame.h"

void VKFrame::setup() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);//VK hates resizing
    window= glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &b_height, &b_height);
}

VKFrame::VKFrame(int _width, int _height, std::string _title) {
    width = _width;
    height = _height;
    title = _title;
    setup();
}

GLFWwindow * VKFrame::getWindow() {
    return window;
}

glm::vec2 VKFrame::getWindowSize() {
    return {width, height};
}

glm::vec2 VKFrame::getBufferSize() {
    return {b_width, b_height};
}

std::string VKFrame::getTitle() {
    return title;
}

void VKFrame::closeInstance() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

VKFrame::~VKFrame() {
   closeInstance();
}
