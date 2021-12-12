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
#include "Triangle.hpp"
#include "FrameBuffer.h"

#include <opencv2/opencv.hpp>

#define PI 3.14
#define TO_RADIAN(x) ((x)*(PI))/180

const int width = 800;
const int height = 800;

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

struct CallbackContext {
    Surface* surface;
    glm::mat4* model;
    Camera* cam;
    Shader* shader;
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
            output.coordinates.push_back(static_cast<float>(j) / (hVertices - 1)); //x texCoordinate
            output.coordinates.push_back(1.0f - static_cast<float>(i) / (vVertices - 1)); //y texCoordinate
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

Ray constructRayThroughPixel(const Camera& camera, int i, int j) {
    const float xNDC = (j + 0.5) / width;
    const float yNDC = (i + 0.5) / height;
    
    const float screenX = 2 * xNDC - 1;
    const float screenY = 1 - 2 * yNDC;
    
    const float aspectRatio = static_cast<float>(width) / height;
    const float cameraX = screenX * aspectRatio * tan(glm::radians(45.f / 2));
    const float cameraY = screenY * tan(glm::radians(45.f / 2));
    
    glm::mat4 inverseCameraMatrix = inverse(camera.GetViewMatrix());
    glm::vec3 camPos = camera.Position;
    glm::vec3 dir = inverseCameraMatrix * glm::vec4(cameraX, cameraY, -1.f, 1.f) - glm::vec4(camPos, 1.0);
    
    return {camPos, glm::normalize(dir)};
}

void rayTrace(const Surface& waterSurface, const mat4& model, const Camera& cam, Shader& sh, int y, int x) {
    for (int i = 0; i < waterSurface.indexCount - 2; ++i) {
        Triangle t;
        if (!(i % 2)) {
            const glm::vec4 v1 = model * glm::vec4(waterSurface.coordinates[waterSurface.indexBuffer.at(i + 1) * 5],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i + 1) * 5 + 1],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i + 1) * 5 + 2],
                                                   1.0);
            
            const glm::vec4 v2 = model * glm::vec4(waterSurface.coordinates[waterSurface.indexBuffer.at(i) * 5],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i) * 5 + 1],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i) * 5 + 2],
                                                   1.0);
            const glm::vec4 v3 = model * glm::vec4(waterSurface.coordinates[waterSurface.indexBuffer.at(i + 2) * 5],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i + 2) * 5 + 1],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i + 2) * 5 + 2],
                                                   1.0);
            
            t.setVertices({v1, v2, v3});
        }
        else {
            const glm::vec4 v1 = model * glm::vec4(waterSurface.coordinates[waterSurface.indexBuffer.at(i + 2) * 5],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i + 2) * 5 + 1],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i + 2) * 5 + 2],
                                                   1.0);
            
            const glm::vec4 v2 = model * glm::vec4(waterSurface.coordinates[waterSurface.indexBuffer.at(i) * 5],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i) * 5 + 1],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i) * 5 + 2],
                                                   1.0);
            
            const glm::vec4 v3 = model * glm::vec4(waterSurface.coordinates[waterSurface.indexBuffer.at(i + 1) * 5],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i + 1) * 5 + 1],
                                                   waterSurface.coordinates[waterSurface.indexBuffer.at(i + 1) * 5 + 2],
                                                   1.0);
            
            t.setVertices({v1, v2, v3});
        }
        
        glm::vec3 n = t.getPlaneNormal();
        
        if (!(isnan(n.x) && isnan(n.y) && isnan(n.z))) {
            float tVal;
            Ray r = constructRayThroughPixel(cam, y, x);
            if (t.intersects(r, tVal)) {
                vec3 intersection = r.p0 + tVal * r.dir;
                std::vector<float> rayAttribs;
                rayAttribs.push_back(r.p0.x);
                rayAttribs.push_back(r.p0.y);
                rayAttribs.push_back(r.p0.z);
                rayAttribs.push_back(intersection.x);
                rayAttribs.push_back(intersection.y);
                rayAttribs.push_back(intersection.z);
                
                glfwSetTime(0.0);
                sh.bind();
                GL_CHECK(glUniform3f(glGetUniformLocation(sh.GetProgramId(), "rippleCenter"), intersection.x, intersection.y, intersection.z));
                std::cout << "Here" << std::endl;
                sh.unbind();
            }
        }
    }

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
    waterMesh.AddLayout(2);
    waterMesh.BindIndexBuffer(waterSurface.indexBuffer.data(), waterSurface.indexCount);
    
    Texture t("/Users/eriknouroyan/Desktop/opengl_project/res/WaterDiffuse.png");

    Shader sh("/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/vertexShader1.vsh",
              "/Users/eriknouroyan/Desktop/opengl_project/shaders/geometry/geometryShader3.gsh",
              "/Users/eriknouroyan/Desktop/opengl_project/shaders/fragment/fragmentShader1.fsh");
    
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
    
    //Grass
    Texture grassDist("/Users/eriknouroyan/Desktop/opengl_project/res/GrassDistribution.png");
    Texture grassText("/Users/eriknouroyan/Desktop/opengl_project/res/GrassDiffuse.png");

    Shader sh3("/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/vertexShader2.vsh",
               "/Users/eriknouroyan/Desktop/opengl_project/shaders/geometry/geometryShader2.gsh",
               "/Users/eriknouroyan/Desktop/opengl_project/shaders/fragment/fragmentShader3.fsh");
    
    Shader lineShader("/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/VshaderTest.vsh",
                      "",
                      "/Users/eriknouroyan/Desktop/opengl_project/shaders/vertex/FshaderTest.fsh");
    
    //Setting up uniforms
    glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, -4.0f)) * glm::scale(glm::mat4(1.0f), glm::vec3(1.0, 1.0, 1.0));
    
    Camera cam;
    
    
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)width / (GLfloat)height, 1.f, 150.0f);
    
    CallbackContext ctx = {&waterSurface, &model, &cam, &sh};
    glfwSetWindowUserPointer(wind, &ctx);
    glfwSetInputMode(wind, GLFW_STICKY_KEYS, 1);
    
    
    auto keyboardCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        CallbackContext* ctx = static_cast<CallbackContext*>(glfwGetWindowUserPointer(window));
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            ctx->cam->ProcessKeyboard(RIGHT, 0.02f);
        }
        
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            ctx->cam->ProcessKeyboard(LEFT, 0.02f);
        }
        
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            ctx->cam->ProcessKeyboard(FORWARD, 0.02f);
        }
        
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            ctx->cam->ProcessKeyboard(BACKWARD, 0.02f);
        }
    };
    glfwSetKeyCallback(wind, keyboardCallback);

    static bool camFreeze=false;

    auto cursorPosCallback = [](GLFWwindow* window, double xpos, double ypos) {
        if (camFreeze ==true) return;
        static float curPosX = xpos, curPosY = ypos;
        if (xpos >= 0 && xpos < width && ypos >= 0 && ypos < height) {
            CallbackContext* ctx = static_cast<CallbackContext*>(glfwGetWindowUserPointer(window));
            float dx = xpos - curPosX;
            float dy = curPosY - ypos;
            ctx->cam->ProcessMouseMovement(dx, dy);
            
            curPosX = xpos;
            curPosY = ypos;
        }
    };
    glfwSetCursorPosCallback(wind, cursorPosCallback);
    
    auto mouseButtonCallback = [](GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) camFreeze=!camFreeze;
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            CallbackContext* ctx = static_cast<CallbackContext*>(glfwGetWindowUserPointer(window));
            double xPos;
            double yPos;
            glfwGetCursorPos(window, &xPos, &yPos);

            std::cout << xPos << " " << yPos << std::endl;
            rayTrace(*(ctx->surface), *(ctx->model), *(ctx->cam), *(ctx->shader), yPos, xPos);
        }
    };
    glfwSetMouseButtonCallback(wind, mouseButtonCallback);
    
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(wind, &framebufferWidth, &framebufferHeight);
    
    
    FrameBuffer fb(width, height);
    
    int defaultFBWidth, defaultFBHeight;
    glfwGetFramebufferSize(wind, &defaultFBWidth, &defaultFBHeight);
    
    while(!glfwWindowShouldClose(wind)){
        fb.Bind();
        //Custom framebuffer viewport setup
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 view = cam.GetInvertedCamera(0.0f);//cam.GetInvertedCamera(2 * (cam.Position.y + 1.5)); // To be changed later to distance to plane
        glEnable(GL_CULL_FACE);
        //Terrain
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
        
        glDisable(GL_CULL_FACE);
        //Grass
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
        
        fb.Unbind();
        glViewport(0, 0, defaultFBWidth, defaultFBHeight);
        
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        view = cam.GetViewMatrix();
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
        //fb.Unbind();
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        t.Bind(GL_TEXTURE0);
        fb.BindTexture(GL_TEXTURE1);
        sh.bind();
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh.GetProgramId(), "view"), 1, GL_FALSE, &view[0][0]));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh.GetProgramId(), "model"), 1, GL_FALSE, &model[0][0]));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh.GetProgramId(), "time"), glfwGetTime()));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(sh.GetProgramId(), "projection"), 1, GL_FALSE, &projection[0][0]));
//        GL_CHECK(glUniform3f(glGetUniformLocation(sh.GetProgramId(), "rippleCenter"), 0.0f, 0.0f, 0.0f));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh.GetProgramId(), "Ka"), 0.3));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh.GetProgramId(), "Kd"), 1.0));
        GL_CHECK(glUniform1f(glGetUniformLocation(sh.GetProgramId(), "Ks"), 1.0));
        GL_CHECK(glUniform3f(glGetUniformLocation(sh.GetProgramId(), "lightDir"), 0.0f, -1.0f, -1.0f));
        GL_CHECK(glUniform3f(glGetUniformLocation(sh.GetProgramId(), "viewDir"), cam.Position.x, cam.Position.y, cam.Position.z));
        glUniform1i(glGetUniformLocation(sh.GetProgramId(), "waterTex"), 0);
        glUniform1i(glGetUniformLocation(sh.GetProgramId(), "reflectionTex"), 1);
        //func(waterSurface, model, cam);

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
