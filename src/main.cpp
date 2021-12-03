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

#include <opencv2/opencv.hpp>

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
            output.indexBuffer.push_back((i + 1) * hVertices + j);
            
            if (i != vVertices - 2 && j == hVertices - 1) {
                output.indexBuffer.push_back((i + 1) * hVertices + j);
                output.indexBuffer.push_back((i + 1) * hVertices);
            }
        }
    }
    
    output.size = output.coordinates.size();
    output.indexCount = output.indexBuffer.size();
    
    return output;
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
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Water
    Surface waterSurface = generateIndexedTriangleStripPlane(100, 100);
    Mesh waterMesh(waterSurface.coordinates.data(), waterSurface.size);
    waterMesh.AddLayout(3);
    waterMesh.AddLayout(2);waterMesh.BindIndexBuffer(waterSurface.indexBuffer.data(), waterSurface.indexCount);

    Texture t("/Users/eriknouroyan/Desktop/opengl_project/res/WaterDiffuse.png");

    Shader sh("/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/vertexShader1.vsh", "", "/Users/eriknouroyan/Desktop/opengl_project/shaders/fragment/fragmentShader1.fsh");
    
    //Terrain
    Surface terrain = generateIndexedTriangleStripPlane(100, 100);
    Mesh terrainMesh(terrain.coordinates.data(), terrain.size);
    terrainMesh.AddLayout(3);
    terrainMesh.AddLayout(2);
    terrainMesh.BindIndexBuffer(terrain.indexBuffer.data(), terrain.indexCount);
    
    Texture t1("/Users/eriknouroyan/Desktop/opengl_project/res/TerrainHeightMap.png");
    Texture t2("/Users/eriknouroyan/Desktop/opengl_project/res/TerrainDiffuse.png");
    
    Shader sh2("/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/vertexShader2.vsh",
               "/Users/eriknouroyan/Desktop/opengl_project/shaders/geometry/geometryShader1.gsh",
               "/Users/eriknouroyan/Desktop/opengl_project/shaders/fragment/fragmentShader2.fsh");
    
    
    Texture grassDist("/Users/eriknouroyan/Desktop/opengl_project/res/GrassDistribution.png");
    Texture grassText("/Users/eriknouroyan/Desktop/opengl_project/res/GrassDiffuse.png");
    
    Shader sh3("/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/vertexShader2.vsh",
               "/Users/eriknouroyan/Desktop/opengl_project/shaders/geometry/geometryShader2.gsh",
               "/Users/eriknouroyan/Desktop/opengl_project/shaders/fragment/fragmentShader3.fsh");
    
    
    //Setting up uniforms
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, -2.0f, -8.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0, 1.0, 1.0));
    Camera cam;
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 1.0f, 150.0f);
    
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
        
        t1.Bind(GL_TEXTURE0);
        t2.Bind(GL_TEXTURE1);
        sh2.bind();

        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh2.GetProgramId(), "view"), 1, GL_FALSE, &view[0][0]));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh2.GetProgramId(), "model"), 1, GL_FALSE, &model[0][0]));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh2.GetProgramId(), "projection"), 1, GL_FALSE, &projection[0][0]));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh2.GetProgramId(), "Ka"), 0.3));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh2.GetProgramId(), "Kd"), 1.0));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh2.GetProgramId(), "Ks"), 1.0));
        GL_CHECK(glUniform3f(glGetUniformLocation(sh2.GetProgramId(), "lightDir"), 0.0f, -1.0f, -1.0f));
        glUniform1i(glGetUniformLocation(sh2.GetProgramId(), "heightMap"), 0);
        glUniform1i(glGetUniformLocation(sh2.GetProgramId(), "terrainTexture"), 1);

        terrainMesh.DrawElements();
        
        
        t.Bind(GL_TEXTURE0);
        sh.bind();
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh.GetProgramId(), "view"), 1, GL_FALSE, &view[0][0]));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh.GetProgramId(), "model"), 1, GL_FALSE, &model[0][0]));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh.GetProgramId(), "time"), glfwGetTime()));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh.GetProgramId(), "projection"), 1, GL_FALSE, &projection[0][0]));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh.GetProgramId(), "Ka"), 0.3));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh.GetProgramId(), "Kd"), 1.0));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh.GetProgramId(), "Ks"), 1.0));
        GL_CHECK(glUniform3f(glGetUniformLocation(sh.GetProgramId(), "lightDir"), 0.0f, -1.0f, -1.0f));
        GL_CHECK(glUniform3f(glGetUniformLocation(sh.GetProgramId(), "viewDir"), cam.Position.x, cam.Position.y, cam.Position.z));
        glUniform1i(glGetUniformLocation(sh.GetProgramId(), "waterTex"), 0);

        waterMesh.DrawElements();
        
        t1.Bind(GL_TEXTURE0);
        grassDist.Bind(GL_TEXTURE1);
        grassText.Bind(GL_TEXTURE2);
        sh3.bind();
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh3.GetProgramId(), "view"), 1, GL_FALSE, &view[0][0]));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh3.GetProgramId(), "model"), 1, GL_FALSE, &model[0][0]));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh3.GetProgramId(), "projection"), 1, GL_FALSE, &projection[0][0]));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh3.GetProgramId(), "Ka"), 0.3));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh3.GetProgramId(), "Kd"), 1.0));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh3.GetProgramId(), "Ks"), 1.0));
        GL_CHECK(glUniform3f(glGetUniformLocation(sh3.GetProgramId(), "lightDir"), 0.0f, -1.0f, -1.0f));
        glUniform1i(glGetUniformLocation(sh3.GetProgramId(), "heightMap"), 0);
        glUniform1i(glGetUniformLocation(sh3.GetProgramId(), "grassDist"), 1);
        glUniform1i(glGetUniformLocation(sh3.GetProgramId(), "grassText"), 2);
        
        terrainMesh.DrawElements();
        
        
        glfwSwapBuffers(wind);
        glfwPollEvents();
    }

    glfwTerminate();
    
    return 0;
}
