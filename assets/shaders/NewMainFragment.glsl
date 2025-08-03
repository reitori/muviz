#version 430 core

in vec2 UV;
in vec3 Normal;
in vec4 Color;

uniform float uColorMixFactor;
uniform sampler2D uTexture;

out vec4 FragColor;

int main(){
    FragColor = mix(texture(uTexture, UV), uColor, uColorMixFactor);
}