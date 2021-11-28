#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <math.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include "Shader.h"
#include "Mesh.h"
#include "Texture.h"

#define PI 3.14
#define TO_RADIAN(x) ((x)*(PI))/180

int main(){
    GLFWwindow* wind;
    if(!glfwInit()){
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    wind = glfwCreateWindow(450, 450, "MyTitle", NULL, NULL);
    if(!wind){
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
    
    
    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);
    
    float vec_positions[16] = {
        -0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, 1.0f, 0.0f
    };
    
    Mesh m(vec_positions, 16);
    m.AddLayout(2);
    m.AddLayout(2);
    
    int indices[4] = {
        0,1,
        2,3
    };
    m.BindIndexBuffer(indices, 4);
    
    Texture t("/Users/eriknouroyan/Desktop/opengl_project/res/WaterDiffuse.png");
    t.Bind(GL_TEXTURE0);
    
    Shader sh("/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/vertexShader1.vsh", "", "/Users/eriknouroyan/Desktop/opengl_project/shaders/fragment/fragmentShader1.fsh");

    sh.bind();
    
    int u_Position = glGetUniformLocation(sh.GetProgramId(), "u_Rotate");
    
    float alpha = 0.0;
    float v[4] = {static_cast<float>(cos(0.418879)), static_cast<float>(-sin(0.418879)), static_cast<float>(sin(0.418879)), static_cast<float>(cos(0.418879))};
    while(!glfwWindowShouldClose(wind)){
        glClear(GL_COLOR_BUFFER_BIT);
        v[0] = static_cast<float>(cosl(TO_RADIAN(alpha)));
        v[1] = static_cast<float>(-sinl(TO_RADIAN(alpha)));
        v[2] = static_cast<float>(sinl(TO_RADIAN(alpha)));
        v[3] = static_cast<float>(cosl(TO_RADIAN(alpha)));
        glUniformMatrix2fv(u_Position, 1, GL_FALSE, v);
        m.DrawElements();
        alpha += 1.0;
        glfwSwapBuffers(wind);
        glfwPollEvents();
    }
    
    glfwTerminate();
    
    return 0;
}
