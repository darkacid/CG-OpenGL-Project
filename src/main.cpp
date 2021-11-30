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
#include "Camera.h"

#define PI 3.14
#define TO_RADIAN(x) ((x)*(PI))/180

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("OpenGL error %i, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

#define GL_CHECK(stmt) do { \
        stmt; \
        CheckOpenGLError(#stmt, __FILE__, __LINE__); \
    } while (0)

struct Surface{
    int size;
    int indexCount;
    std::vector<float> coordinates;
    std::vector<int> indexBuffer;
};

Surface generateIndexedTriangleStripPlane(int hVertices, int vVertices) {
    float dH = 2.f / (hVertices - 1);
    float dV = 2.f / (vVertices - 1);
    
    Surface output;
    for (int i = 0; i < vVertices; ++i) {
        for (int j = 0; j < hVertices ; ++j) {
            output.coordinates.push_back(j * dH - 1.0f); //x
            output.coordinates.push_back(0.5); //y
            output.coordinates.push_back(i * dV - 1.0f); //z
            output.coordinates.push_back(static_cast<float>(j) / hVertices); //x texCoordinate
            output.coordinates.push_back(1.0f - static_cast<float>(i) / vVertices); //y texCoordinate
        }
    }
    
    for (int i = 0; i < vVertices - 1; ++i) {
        for (int j = 0; j < hVertices; ++j) {
            output.indexBuffer.push_back(i * hVertices + j);
            output.indexBuffer.push_back((i + 1) * hVertices +j);
        }
    }
    output.size = output.coordinates.size();
    output.indexCount = output.indexBuffer.size();
    
    return output;
}

void glDebugCallback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam)
{
    std::cout << "[OpenGL Error](" << type << ") " << message << std::endl;
}

int main(){
    GLFWwindow* wind;
    if(!glfwInit()){
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    const int width = 800;
    const int height = 800;
    
    wind = glfwCreateWindow(width, height, "MyTitle", NULL, NULL);
    
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
    glDebugMessageCallback(glDebugCallback,0);

//    Surface waterSurface = generateIndexedTriangleStripPlane(10, 7);
//
//    Mesh m(waterSurface.coordinates.data(), waterSurface.size);
//    m.AddLayout(3);
//    m.AddLayout(2);
//
//    m.BindIndexBuffer(waterSurface.indexBuffer.data(), waterSurface.indexCount);
//
//    Texture t("/Users/eriknouroyan/Desktop/opengl_project/res/WaterDiffuse.png");
//    t.Bind(GL_TEXTURE0);
//
//    Shader sh("/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/vertexShader1.vsh", "", "/Users/eriknouroyan/Desktop/opengl_project/shaders/fragment/fragmentShader1.fsh");
//
//    sh.bind();
    
    Surface terrain = generateIndexedTriangleStripPlane(10, 7);
    Mesh m(terrain.coordinates.data(), terrain.size);
    m.AddLayout(3);
    m.AddLayout(2);

    m.BindIndexBuffer(terrain.indexBuffer.data(), terrain.indexCount);


    Shader sh2("../shaders/vertex/vertexShader2.vsh",
               "",//"/Users/eriknouroyan/Desktop/opengl_project/shaders/geometry/geometryShader1.gsh"
               "../shaders/fragment/fragmentShader2.fsh");

    sh2.bind();

    Texture t1("../res/TerrainHeightMap.png");
    t1.Bind(GL_TEXTURE0);
    
    Texture t2("../res/TerrainDiffuse.png");
    t2.Bind(GL_TEXTURE1);

    
    
    //Setting up uniforms
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -8.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0, 1.0, 4.0));
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh2.GetProgramId(), "model"), 1, GL_FALSE, &model[0][0]));
    
    Camera cam;
    glm::mat4 view = cam.GetViewMatrix();
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh2.GetProgramId(), "view"), 1, GL_FALSE, &view[0][0]));
    
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 1.0f, 150.0f);
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh2.GetProgramId(), "projection"), 1, GL_FALSE, &projection[0][0]));
    
    int kALocation = glGetUniformLocation(sh2.GetProgramId(), "Ka");
    GL_CHECK(glUniform1f(kALocation, 0.1));
    
    int kDLocation = glGetUniformLocation(sh2.GetProgramId(), "Kd");
    GL_CHECK(glUniform1f(kDLocation, 1.0));
    
    int kSLocation = glGetUniformLocation(sh2.GetProgramId(), "Ks");
    GL_CHECK(glUniform1f(kSLocation, 1.0));
    
    GL_CHECK(glUniform3f(glGetUniformLocation(sh2.GetProgramId(), "lightDir"), -0.2f, -1.0f, -0.3f));
    
    glfwSetWindowUserPointer(wind, &cam);
    glfwSetInputMode(wind, GLFW_STICKY_KEYS, 1);
    
    
    auto keyboardCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (key == GLFW_KEY_RIGHT && action == GLFW_REPEAT)
        {
            cam->ProcessKeyboard(RIGHT, 0.02f);
        }
        else if (key == GLFW_KEY_LEFT && action == GLFW_REPEAT) {
            cam->ProcessKeyboard(LEFT, 0.02f);
        }
        else if (key == GLFW_KEY_UP && action == GLFW_REPEAT) {
            cam->ProcessKeyboard(FORWARD, 0.02f);
        }
        else if (key == GLFW_KEY_DOWN && action == GLFW_REPEAT) {
            cam->ProcessKeyboard(BACKWARD, 0.02f);
        }
    };
    
    glfwSetKeyCallback(wind, keyboardCallback);
    
    auto cursorPosCallback = [](GLFWwindow* window, double xpos, double ypos) {
        static float curPosX = xpos, curPosY = ypos;
        if (xpos >= 0 && xpos < width && ypos >= 0 && ypos < height) {
            Camera* cam = static_cast<Camera*>(glfwGetWindowUserPointer(window));
            float dx = xpos - curPosX;
            float dy = curPosY - ypos;
            cam->ProcessMouseMovement(dx, dy);
            
            curPosX = xpos;
            curPosY = ypos;
        }
    };
    glfwSetCursorPosCallback(wind, cursorPosCallback);
    
    while(!glfwWindowShouldClose(wind)){
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        glm::mat4 view = cam.GetViewMatrix();
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh2.GetProgramId(), "view"), 1, GL_FALSE, &view[0][0]));
        m.DrawElements();

        GL_CHECK(glUniform1f(glGetUniformLocation(sh2.GetProgramId(), "time"), glfwGetTime()));
        
        glfwSwapBuffers(wind);
        glfwPollEvents();
    }

    glfwTerminate();
    
    return 0;
}
