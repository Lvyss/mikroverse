#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D textTexture;
uniform float alpha;
uniform float time;
uniform float glowIntensity;

void main() {
    vec4 texColor = texture(textTexture, TexCoord);
    
    // Efek pulsing (naik turun alpha/glow)
    float pulse = 0.7 + sin(time * 2.0) * 0.2;
    
    // Efek scanline hologram
    float scanline = sin(TexCoord.y * 800.0 + time * 10.0) * 0.15;
    
    // Efek rainbow edge (chromatic)
    float edgeGlow = 0.0;
    if (TexCoord.x < 0.05 || TexCoord.x > 0.95 || TexCoord.y < 0.05 || TexCoord.y > 0.95) {
        edgeGlow = 0.8;
    }
    
    // Efek grid hologram
    float gridX = abs(fract(TexCoord.x * 20.0) - 0.5) * 2.0;
    float gridY = abs(fract(TexCoord.y * 20.0) - 0.5) * 2.0;
    float grid = (1.0 - smoothstep(0.7, 0.9, gridX)) * (1.0 - smoothstep(0.7, 0.9, gridY));
    grid = clamp(grid * 0.3, 0.0, 0.2);
    
    // Warna hologram dengan efek
    vec3 hologramColor = texColor.rgb;
    
    // Efek warna berubah pelan (cyan ke ungu)
    vec3 color1 = vec3(0.2, 0.8, 1.0); // cyan
    vec3 color2 = vec3(0.8, 0.3, 1.0); // ungu
    float t = (sin(time * 0.5) + 1.0) / 2.0;
    vec3 tintColor = mix(color1, color2, t);
    
    // Kombinasi semua efek
    float finalAlpha = texColor.a * alpha * pulse;
    vec3 finalColor = hologramColor * tintColor * (0.8 + glowIntensity + scanline + edgeGlow + grid);
    
    FragColor = vec4(finalColor, finalAlpha);
}