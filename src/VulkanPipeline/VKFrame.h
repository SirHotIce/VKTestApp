//
// Created by Vinayak Regmi on 08/06/2025.
//

#ifndef VKFRAME_H
#define VKFRAME_H
#include <string>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <stdexcept>

class VKFrame {

private:
    uint32_t width, height;
    int b_width, b_height;
    std::string title;
    GLFWwindow* window;
    void setup();
public:
    VKFrame(int _width, int _height, std::string _title);
    GLFWwindow* getWindow();
    glm::vec2 getWindowSize();
    glm::vec2 getBufferSize();
    std::string getTitle();
    void closeInstance();
    ~VKFrame();
};



#endif //VKFRAME_H
