//
//  Triangle.cpp
//  Test
//
//  Created by Erik Nouroyan on 19.11.21.
//

#include "Triangle.hpp"
#include <iostream>

Triangle::Triangle(): vertices{{1.f, 0.f, -1.f}, {-1.f, 0.f, -1.f}, {0.f, 1.f, -1.f}}, color{128, 128, 128} {}

Triangle::Triangle(const std::initializer_list<Vertex>& il, const Color& c) {
    if (il.size() != 3) {
        std::cout << "Invalid size for initializer list!" << std::endl;
    }
    vertices = std::vector<Vertex>(3);
    std::copy(il.begin(), il.end(), vertices.begin());
    setColor(c);
}

void Triangle::setColor(const Color& col) {
    color = col;
}

Color Triangle::getColor() const {
    return color;
}

void Triangle::setVertices(const std::vector<Vertex>& vertices) {
    if (vertices.size() != 3) {
        std::cout << "Invalid size for vertices!" << std::endl;
    }
    this->vertices = vertices;
}

std::vector<Vertex> Triangle::getVertices() const {
    return vertices;
}

glm::vec3 Triangle::getPlaneNormal() const {
    return glm::normalize(glm::cross(vertices[1] - vertices[0], vertices[2] - vertices[0]));
}

float Triangle::getPlaneDistance() const {
    return abs(glm::dot(vertices[0], getPlaneNormal()));
}

bool Triangle::isInsideTriangle(const glm::vec3& point) const {
    glm::vec3 normal = getPlaneNormal();
    for (int i = 0; i < 3; ++i) {
        glm::vec3 edge = vertices[(i + 1) % 3] - vertices[i];
        glm::vec3 vertexToPoint = point - vertices[i];
        glm::vec3 cross = glm::cross(edge, vertexToPoint);
        if (glm::dot(cross, normal) < 0.f) {
            return false;
        }
    }
    
    return true;
}

bool Triangle::intersects(const Ray& ray, float& t) const {
    float dist = getPlaneDistance();
    glm::vec3 normal = getPlaneNormal();
    float dirDotNormal = glm::dot(ray.dir, normal);
    
    //Parallel case
    if (abs(dirDotNormal - 0.f) < 0.0001) {
        return false;
    }
    
    float tTmp = (dist - dot(ray.p0, normal)) / dirDotNormal;
    
    //Ray is behind
    if (tTmp < 0.f) {
        return false;
    }
    
    glm::vec3 intersectionPoint = ray.p0 + tTmp * ray.dir;
    if (!isInsideTriangle(intersectionPoint)) {
        return false;
    }
    
    t = tTmp;
    
    return true;
}
