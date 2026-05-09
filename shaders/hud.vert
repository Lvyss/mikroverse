#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aUV;

uniform mat4 projection;
uniform mat4 model;

out vec2 TexCoords;
out vec2 FragPos;    // posisi lokal dalam rect (0..1)

void main() {
    TexCoords = aUV;
    FragPos   = aPos;
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
}