#version 330 core
out vec4 FragColor;
in  vec2 TexCoords;
uniform sampler2D screenTexture;
uniform int effect;

void main() {
    vec2 uv  = TexCoords;
    vec3 base = texture(screenTexture, uv).rgb;

    // ── Effect 0: passthrough ─────────────────────────────
    if (effect == 0) {
        FragColor = vec4(base, 1.0);
        return;
    }

    // ── Effect 4 / default: Bloom + vignette ─────────────
    // Ekstrak pixel sangat terang (neon lines)
    vec3 bloom = vec3(0.0);
    float wSum = 0.0;
    vec2  texel = 1.0 / vec2(1280.0, 720.0);

    // Two-pass approximation: blur radius 5
    for (int x = -5; x <= 5; x++) {
    for (int y = -5; y <= 5; y++) {
        float w   = exp(-float(x*x + y*y) * 0.10);
        vec3  s   = texture(screenTexture, uv + vec2(x,y)*texel*2.0).rgb;
        // Threshold: hanya pixel yang sangat bright (neon)
        float lum = dot(s, vec3(0.2126, 0.7152, 0.0722));
        float t   = smoothstep(0.45, 0.90, lum);
        bloom    += s * w * t;
        wSum     += w;
    }}
    bloom /= wSum;

    // Chromatic aberration pada bloom
    float str = 0.0025;
    vec2  dir = normalize(uv - 0.5 + vec2(0.001)) * str;
    vec3 bloomAberr;
    bloomAberr.r = dot(
        texture(screenTexture, uv + dir*1.5).rgb,
        vec3(0.2126,0.7152,0.0722)) > 0.45
        ? texture(screenTexture, uv + dir*1.5).r : 0.0;
    bloomAberr.g = bloom.g;
    bloomAberr.b = dot(
        texture(screenTexture, uv - dir*1.5).rgb,
        vec3(0.2126,0.7152,0.0722)) > 0.45
        ? texture(screenTexture, uv - dir*1.5).b : 0.0;

    // Mix bloom ke base — neon glow effect
    vec3 col = base + mix(bloom, bloomAberr, 0.5) * 0.55;

    // Vignette (darkness di corner, dramatik)
    vec2  vc   = uv - 0.5;
    float vig  = 1.0 - dot(vc, vc) * 1.8;
    col       *= clamp(vig, 0.0, 1.0);

    // Subtle contrast boost
    col = pow(col, vec3(0.95));
    col = (col - 0.5) * 1.08 + 0.5;

    // ── Effect 1: Chromatic aberration only ──────────────
    if (effect == 1) {
        float s2  = 0.004;
        vec2  d2  = (uv - 0.5) * s2;
        float r   = texture(screenTexture, uv + d2).r;
        float g   = texture(screenTexture, uv).g;
        float b   = texture(screenTexture, uv - d2).b;
        col = vec3(r, g, b);
    }

    FragColor = vec4(clamp(col, 0.0, 1.0), 1.0);
}