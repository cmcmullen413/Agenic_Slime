#version 330 core

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec2 texPos;

out vec2 texCoord;

void main() {
    gl_Position = vec4(vPos, 1.0f, 1.0f);
    texCoord = texPos;
}