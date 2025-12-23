#version 330 core

layout (location = 0) in vec3 aPos;

uniform vec4 uColor;
uniform mat4 uView;
uniform mat4 uProj;

out vec4 fragOut;

void main()
{
   gl_Position = uProj * uView * vec4(aPos, 1.0);
   fragOut = uColor;
}


