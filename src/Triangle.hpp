//
//  Triangle.hpp
//  Test
//
//  Created by Erik Nouroyan on 19.11.21.
//

#ifndef Triangle_hpp
#define Triangle_hpp

#include <initializer_list>
#include <vector>
#include <glm/mat4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <opencv2/opencv.hpp>

using Color = cv::Vec3b;
using Vertex = glm::vec3;

struct Ray {
    glm::vec3 p0;
    glm::vec3 dir;
};

class Triangle {
public:
    Triangle();
    Triangle(const std::initializer_list<Vertex>& il, const Color& c);
    void setColor(const Color& col);
    Color getColor() const;
    void setVertices(const std::vector<Vertex>& vertices);
    std::vector<Vertex> getVertices() const;
    bool intersects(const Ray& r, float& t) const;
    
private:
    std::vector<Vertex> vertices;
    Color color;
public:
    glm::vec3 getPlaneNormal() const;
    float getPlaneDistance() const;
public:
    bool isInsideTriangle(const glm::vec3& point) const;
};

#endif /* Triangle_hpp */
