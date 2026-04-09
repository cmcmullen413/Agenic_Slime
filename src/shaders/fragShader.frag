#version 330 core

in vec2 texCoord;

out vec4 FragColor;

uniform sampler2D tex;

void main() {
    float strength = texture(tex, texCoord).r;
    FragColor = vec4(strength*vec3(1.f, 1.f, 1.f), 1.f);
}