#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 tCoord;

out vec2 texCoord;
uniform mat2 u_Rotate;

void main(){
    gl_Position = vec4(u_Rotate * position, -1.0, 1.0);
    texCoord = tCoord;
}
