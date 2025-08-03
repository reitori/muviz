#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aFrag;

layout (location = 2) in vec4 aInstanceFrag;
layout (location = 3) in mat4 aInstanceModel;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform bool uIsInstanced;
uniform bool uUseGlobalColor;
uniform vec4 uGlobalColor;

out vec4 fragOut;

void main()
{
   mat4 modelTransform = uIsInstanced ? aInstanceModel : uModel;
   gl_Position = uProj * uView * modelTransform * vec4(aPos, 1.0);
   fragOut = uUseGlobalColor ? uGlobalColor : (uIsInstanced ? aInstanceFrag : aFrag);
}

