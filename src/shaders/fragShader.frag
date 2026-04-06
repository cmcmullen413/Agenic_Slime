#version 330 core

in vec4 fColor;
in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex;

void main() {
    FragColor = texture(tex, texCoord) * fColor;
}