#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 aUV;

layout (location = 3) in vec4 aInstanceColor;
layout (location = 4) in mat4 aInstanceModel;

uniform mat4 uView;
uniform mat4 uProj;

out vec2 UV;
out vec3 Normal;
out vec3 Color;

void main(){
    gl_Position = uProj * uView * aInstanceModel * aPos;
    UV = aUV;
    Normal = aNormal;
    Color = aInstanceColor; 
}