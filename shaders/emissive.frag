#version 330 core
out vec4 FragColor;
uniform vec3  emissiveColor;
uniform float emissiveStrength;

void main() {
    vec3 col = emissiveColor * emissiveStrength;
    // Pure white-hot core — biar bloom nangkep maksimal
    float whiteness = smoothstep(2.0, 5.0, emissiveStrength);
    col = mix(col, vec3(emissiveStrength), whiteness * 0.6);
    FragColor = vec4(col, 1.0);
}