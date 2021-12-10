#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoord;
} gs_in[];

out vec2 texCoords;
out vec3 interPos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 rippleCenter;
uniform float time;

vec3 getNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(b, a));
}

void main(){
    normal = transpose(inverse(mat3(model))) * getNormal();
    
    vec4 pos = gl_in[0].gl_Position;
    float dist = distance(vec3(model * pos), rippleCenter);
    float offset = 10 * dist / (time + 1) * pow(2, -9.0 * dist) * cos(44 * dist - 3 * time);
    gl_Position = projection * view * model * vec4(pos.x, pos.y + offset, pos.z, 1.0);
    interPos = transpose(inverse(mat3(model))) * vec3(pos.x, pos.y + offset, pos.z);
    texCoords = gs_in[0].texCoord;
    EmitVertex();
    
    pos = gl_in[1].gl_Position;
    dist = distance(vec3(model * pos), rippleCenter);
    offset = 10 * dist / (time + 1) * pow(2, -9.0 * dist) * cos(44 * dist - 3 * time);
    gl_Position = projection * view * model * vec4(pos.x, pos.y + offset, pos.z, 1.0);
    interPos = transpose(inverse(mat3(model))) * vec3(pos.x, pos.y + offset, pos.z);
    texCoords = gs_in[1].texCoord;
    EmitVertex();
    
    pos = gl_in[2].gl_Position;
    dist = distance(vec3(model * pos), rippleCenter);
    offset = 10 * dist / (time + 1) * pow(2, -9.0 * dist) * cos(44 * dist - 3 * time);
    gl_Position = projection * view * model * vec4(pos.x, pos.y + offset, pos.z, 1.0);
    interPos = transpose(inverse(mat3(model))) * vec3(pos.x, pos.y + offset, pos.z);
    texCoords = gs_in[2].texCoord;
    EmitVertex();
    EndPrimitive();
}
