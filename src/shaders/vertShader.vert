#version 330 core

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec3 vColor;
layout (location = 2) in vec2 texPos;

out vec4 fColor;
out vec2 texCoord;

void main() {
    gl_Position = vec4(vPos, 1.0f, 1.0f);
    fColor = vec4(vColor, 1.f);
    texCoord = texPos;
}