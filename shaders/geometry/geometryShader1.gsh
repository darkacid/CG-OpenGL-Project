#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoord;
} gs_in[];

out vec2 texCoords;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

vec3 getNormal()
{
   vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
   vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[1].gl_Position);
   return normalize(cross(b, a));
}

void main(){
    normal = transpose(inverse(mat3(model))) * getNormal();
    
    gl_Position = projection * view * model * gl_in[0].gl_Position;
    texCoords = gs_in[0].texCoord;
    EmitVertex();
    
    //normal = transpose(inverse(mat3(view * model))) * getNormal();
    gl_Position = projection * view * model * gl_in[1].gl_Position;
    texCoords = gs_in[1].texCoord;
    EmitVertex();
    
    //normal = transpose(inverse(mat3(view * model))) * getNormal();
    gl_Position = projection * view * model * gl_in[2].gl_Position;
    texCoords = gs_in[2].texCoord;
    EmitVertex();
    EndPrimitive();
}
