#version 330 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex;

void main() {
    if (texture(tex, texCoord) == vec4(1.f, 0.f, 0.f, 1.f)) {
        FragColor = vec4(1.f, 1.f, 1.f, 1.f);
    }
    else {
        FragColor = vec4(0.f, 0.f, 0.f, 1.f);
    }
}