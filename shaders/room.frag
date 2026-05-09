#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 WorldPos;

uniform vec3 viewPos;

#define MAX_LIGHTS 8
uniform vec3  lightPos[MAX_LIGHTS];
uniform vec3  lightColor[MAX_LIGHTS];
uniform float lightRadius[MAX_LIGHTS];
uniform int   numLights;

float hash(vec2 p) {
    p = fract(p * vec2(234.34, 435.345));
    p += dot(p, p + 34.23);
    return fract(p.x * p.y);
}
float noise(vec2 p) {
    vec2 i = floor(p); vec2 f = fract(p);
    f = f*f*(3.0-2.0*f);
    return mix(mix(hash(i),hash(i+vec2(1,0)),f.x),
               mix(hash(i+vec2(0,1)),hash(i+vec2(1,1)),f.x),f.y);
}
float fbm(vec2 p) {
    float v=0.0,a=0.5;
    for(int i=0;i<4;i++){v+=a*noise(p);p*=2.1;a*=0.5;}
    return v;
}

// ── Lantai hitam glossy dengan reflection neon ────────────
// ── Lantai: dark bata tidak beraturan + retakan ───────────
vec3 darkBrickFloor(vec2 uv, vec3 norm, vec3 viewDir) {
    // Skala bata besar, tidak seragam
    vec2 scaled = uv * vec2(0.45, 0.30);
    vec2 cell   = floor(scaled);
    vec2 local  = fract(scaled);

    // Offset baris ganjil (bond pattern)
    float rowOdd = mod(cell.y, 2.0);
    vec2  shiftedCell  = floor(vec2(scaled.x + rowOdd*0.5, scaled.y));
    vec2  shiftedLocal = fract(vec2(scaled.x + rowOdd*0.5, scaled.y));

    float rnd  = hash(shiftedCell);
    float rnd2 = hash(shiftedCell + vec2(3.7, 9.1));
    float rnd3 = hash(shiftedCell * vec2(7.3, 2.9));

    // Ukuran bata bervariasi (tidak beraturan)
    float sX = 0.50 + rnd  * 0.40;
    float sY = 0.55 + rnd2 * 0.38;
    float dX = abs(shiftedLocal.x - 0.5) / sX;
    float dY = abs(shiftedLocal.y - 0.5) / sY;
    float edgeDist = max(dX, dY);

    // Mortar/celah tebal dan gelap
    float mortar = smoothstep(0.82, 0.96, edgeDist);

    // Warna bata: charcoal sangat gelap
    float brickBase = 0.06 + rnd3 * 0.05 + fbm(uv * 3.5 + rnd * 7.0) * 0.04;
    vec3  brickCol  = vec3(brickBase) * vec3(0.82, 0.86, 0.90);

    // Crack/retakan di dalam bata
    float crackNoise = fbm(uv * 12.0 + rnd2 * 5.0);
    float crackLine  = abs(crackNoise - 0.52);
    float crack      = smoothstep(0.018, 0.0, crackLine) * (1.0 - mortar) * rnd3;
    brickCol        -= crack * 0.04;

    // Bevel tipis di tepi bata
    float bevel = smoothstep(0.70, 0.82, edgeDist) * (1.0-mortar) * 0.03;
    brickCol   += bevel;

    // Mortar warna gelap total
    vec3 mortarCol = vec3(0.018, 0.020, 0.028);
    vec3 col       = mix(brickCol, mortarCol, mortar);

    // Minimal reflection — hanya di mortar, sangat subtle
    float fresnel = pow(1.0 - max(dot(norm, viewDir), 0.0), 5.0);
    vec3  nRef    = vec3(0.015, 0.035, 0.10) * fresnel * (1.0 - mortar) * 0.4;

    return col + nRef;
}

// ── Dinding: panel tech acak ──────────────────────────────
vec3 techWall(vec2 uv) {
    vec2 pCell = floor(uv * vec2(1.1, 0.85));
    vec2 pLoc  = fract(uv * vec2(1.1, 0.85));
    float rnd  = hash(pCell);
    float rnd2 = hash(pCell * 2.71 + vec2(1.3,4.7));

    float sX = 0.58 + rnd  * 0.34;
    float sY = 0.58 + rnd2 * 0.34;
    float panelEdge = max(abs(pLoc.x-0.5)/sX, abs(pLoc.y-0.5)/sY);
    float groove    = smoothstep(0.85, 0.97, panelEdge);

    // Sub-lines internal per panel
    float subU = fract(uv.x * 4.2 + rnd * 9.0);
    float subV = fract(uv.y * 2.8 + rnd2 * 7.0);
    float sub  = (smoothstep(0.93,1.0,subU) + smoothstep(0.93,1.0,subV)) * 0.5;

    float conc     = fbm(uv * 8.0 + rnd * 5.0) * 0.04;
    float bright   = 0.07 + rnd * 0.06 + conc;
    // Panel recessed acak
    bright        *= (1.0 - step(0.70, rnd2) * 0.40);
    vec3  panelCol = vec3(bright) * vec3(0.78, 0.84, 0.97);
    panelCol      += smoothstep(0.78, 0.86, panelEdge) * (1.0-groove) * 0.05;
    panelCol      -= sub * 0.02 * (1.0-groove);

    return clamp(mix(panelCol, vec3(0.025,0.028,0.04), groove), 0.0, 1.0);
}

// ── Langit-langit: panel dengan channel glow ─────────────
vec3 techCeiling(vec2 uv) {
    vec2 gID  = floor(uv * vec2(1.7, 1.05));
    vec2 gLoc = fract(uv * vec2(1.7, 1.05));
    float rnd  = hash(gID);
    float rnd2 = hash(gID + vec2(5.3, 2.9));

    float edge    = max(abs(gLoc.x-0.5), abs(gLoc.y-0.5));
    float frame   = smoothstep(0.43, 0.50, edge);
    float channel = smoothstep(0.38,0.43,edge) *
                    smoothstep(0.50,0.43,edge) *
                    step(0.60, rnd2);

    float baseGray = 0.06 + rnd*0.05 + fbm(uv*6.0)*0.025;
    baseGray      *= (1.0 - step(0.52, rnd) * 0.42);
    vec3 col       = vec3(baseGray) * vec3(0.77, 0.83, 0.97);
    col            = mix(col, vec3(0.022,0.025,0.038), frame);
    col            = mix(col, vec3(0.12,0.22,0.50)*0.55, channel*(1.0-frame));
    return col;
}

float calcSpec(vec3 n, vec3 l, vec3 v, float sh) {
    return pow(max(dot(n, normalize(l+v)), 0.0), sh);
}

void main() {
    vec3 norm    = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    bool isFloor   = norm.y >  0.8;
    bool isCeiling = norm.y < -0.8;

    vec3  albedo;
    float shininess, specStr;

if (isFloor) {
    albedo    = darkBrickFloor(WorldPos.xz * 0.28, norm, viewDir);
    shininess = 25.0;   // bata tidak glossy
    specStr   = 0.08;
}else if (isCeiling) {
        albedo    = techCeiling(WorldPos.xz * 0.13);
        shininess = 12.0;
        specStr   = 0.06;
    } else {
        vec2 uv   = abs(norm.x) > 0.8
                    ? WorldPos.zy * 0.11
                    : WorldPos.xy * 0.11;
        albedo    = techWall(uv);
        shininess = 50.0;
        specStr   = 0.25;
    }

    // ── Ambient sangat gelap ──────────────────────────────
    vec3 result = 0.018 * albedo;
    // Neon bleed ambient (biru sangat subtle)
    result += vec3(0.006, 0.010, 0.022);

    // ── Point lights ─────────────────────────────────────
    for (int i = 0; i < numLights && i < MAX_LIGHTS; i++) {
        vec3  lDir  = normalize(lightPos[i] - FragPos);
        float dist  = length(lightPos[i] - FragPos);
        float atten = lightRadius[i] / (1.0 + 0.14*dist + 0.07*dist*dist);
        float diff  = max(dot(norm, lDir), 0.0);
        float spec  = calcSpec(norm, lDir, viewDir, shininess) * specStr;
        result     += atten * lightColor[i] * (diff * albedo + vec3(spec));
    }

    // ── Tone mapping + gamma ──────────────────────────────
    result  = result / (result + vec3(1.0));
    result  = pow(result, vec3(1.0/2.2));
    FragColor = vec4(result, 1.0);
}