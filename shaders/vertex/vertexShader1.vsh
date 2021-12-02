#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 tCoord;

out vec2 texCoord;
out vec3 interPos;
out vec3 interNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(position, 1.0f);
    interPos = vec3(model * vec4(position, 1.0f));
    interNormal = transpose(inverse(mat3(view * model))) * vec3(0.0, 1.0, 0.0);
    texCoord = tCoord;
}
