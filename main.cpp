//
//  main.cpp
//  CGFinalProject
//
//  Created by Erik Nouroyan on 27.11.21.
//
#include <GL/glew.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include "Texture.h"

int main(int argc, const char * argv[]) {
    GLFWwindow* wind;
    if (!glfwInit()) {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    const int width = 700;
    const int height = 700;
    
    
    wind = glfwCreateWindow(width, height, "MyTitle", NULL, NULL);
    if (!wind) {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(wind);
    
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    Texture t("/Users/eriknouroyan/Desktop/opengl_project/res/WaterDiffuse.png");
    
    std::cout << "Hello, World!\n";
    return 0;
}
