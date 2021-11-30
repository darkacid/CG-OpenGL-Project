#version 330 core

in vec2 texCoords;
in vec3 normal;

out vec4 color;

uniform float Ka;
uniform float Kd;
uniform float Ks;

uniform vec3 lightDir;
uniform sampler2D heightMap;
uniform sampler2D terrainTexture;

void main(){
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 Ia = Ka * lightColor;
    vec3 lightDirection = normalize(-lightDir);
    float lambertian = max(dot(lightDirection, normalize(normal)), 0.0);
    vec3 Id = Kd * lambertian * lightColor;
//    float spec = 0.0;
//    vec3 Is = vec3(0.0);
//    if(lambertian > 0.0) {
//      vec3 reflection = reflect(-lightDir, interNormal);
//      vec3 view = normalize(viewDirection - interPosition);
//      float specAngle = max(dot(reflection, view), 0.0);
//      Is = Ks * pow(specAngle, 120.f) * specularColor;
//    }

    color = normalize(vec4(Ia + Id , 1.0) * texture(heightMap, texCoords));    //+ Is
    color = texture(heightMap, texCoords);
}
