#version 330 core
in vec2 TexCoords;

uniform sampler2D text;
uniform vec3  textColor;
uniform float textAlpha;

out vec4 FragColor;

void main() {
    float a = texture(text, TexCoords).r;
    FragColor = vec4(textColor, a * textAlpha);
}