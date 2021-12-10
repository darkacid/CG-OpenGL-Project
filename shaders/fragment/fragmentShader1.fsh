#version 330 core

in vec2 texCoords;
in vec3 interPos;
in vec3 normal;

out vec4 color;

uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float time;

uniform vec3 lightDir;
uniform vec3 viewDir;
uniform sampler2D waterTex;
uniform sampler2D reflectionTex;

void main(){
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 Ia = Ka * lightColor;
    vec3 lightDirection = normalize(-lightDir);
    float lambertian = max(dot(lightDirection, normalize(normal)), 0.0);
    vec3 Id = Kd * lambertian * lightColor;
    float spec = 0.0;
    vec3 Is = vec3(0.0);
    if(lambertian > 0.0) {
      vec3 reflection = reflect(-lightDir, normal);
      vec3 view = normalize(viewDir - interPos);
      float specAngle = max(dot(reflection, view), 0.0);
      Is = Ks * pow(specAngle, 120.f) * lightColor;
    }

    vec2 textureCoord = vec2(texCoords.x + time / 8, texCoords.y);
    vec2 ss = interPos.xz * 0.5 + 0.5;
    vec2 reflection = vec2(ss.x, 1.0 -ss.y);
    color = normalize(vec4(Ia + Id + Is, 1.0) * mix(texture(waterTex, textureCoord), texture(reflectionTex, reflection), 0.7));
    //color = texture(reflectionTex, texCoords);
}
