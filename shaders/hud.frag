#version 330 core
in vec2 TexCoords;
in vec2 FragPos;    // 0..1 local space

uniform vec4  color;
uniform float radius;    // corner radius dalam pixel
uniform vec2  rectSize;  // ukuran rect dalam pixel

out vec4 FragColor;

// SDF rounded box
float roundedBox(vec2 uv, vec2 size, float r) {
    // Konversi uv (0..1) ke pixel coords dari center
    vec2 p = (uv - 0.5) * size;
    vec2 q = abs(p) - (size * 0.5 - r);
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

void main() {
    float alpha = color.a;

    if (radius > 0.5 && rectSize.x > 1.0 && rectSize.y > 1.0) {
        float d = roundedBox(FragPos, rectSize, radius);
        // Smooth edge
        alpha *= 1.0 - smoothstep(-1.0, 1.0, d);
    }

    if (alpha < 0.005) discard;
    FragColor = vec4(color.rgb, alpha);
}