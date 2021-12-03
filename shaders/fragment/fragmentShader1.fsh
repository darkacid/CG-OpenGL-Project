#version 330 core

in vec2 texCoord;
in vec3 interPos;
in vec3 interNormal;

out vec4 color;

uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float time;

uniform vec3 lightDir;
uniform vec3 viewDir;
uniform sampler2D waterTex;

void main(){
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    vec3 Ia = Ka * lightColor;
    vec3 lightDirection = normalize(-lightDir);
    float lambertian = max(dot(lightDirection, normalize(interNormal)), 0.0);
    vec3 Id = Kd * lambertian * lightColor;
    float spec = 0.0;
    vec3 Is = vec3(0.0);
    if(lambertian > 0.0) {
      vec3 reflection = reflect(-lightDir, interNormal);
      vec3 view = normalize(viewDir - interPos);
      float specAngle = max(dot(reflection, view), 0.0);
      Is = Ks * pow(specAngle, 120.f) * lightColor;
    }

    vec2 textureCoord = vec2(texCoord.x + time / 8, texCoord.y);
    color = normalize(vec4(Ia + Id + Is, 1.0) * texture(waterTex, textureCoord));    //+ Is
}
