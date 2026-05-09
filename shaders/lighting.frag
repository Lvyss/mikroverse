#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 FragPos;
in vec3 Normal;

// Textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;

// Render mode
uniform int renderMode;

// Camera
uniform vec3 viewPos;

// Point lights dari lab (neon/spotlight)
#define MAX_LIGHTS 4
uniform vec3  lightPositions[MAX_LIGHTS];
uniform vec3  lightColors[MAX_LIGHTS];
uniform float lightIntensities[MAX_LIGHTS];
uniform int   lightCount;

// Spotlight
struct SpotLight {
    vec3  position;
    vec3  direction;
    vec3  color;
    float cutOff;
    float outerCutOff;
    float intensity;
};
uniform SpotLight spotLights[4];
uniform int       spotLightCount;

// ── UV Checker ────────────────────────────────────────────
vec3 uvChecker(vec2 uv, float scale) {
    vec2 s = floor(uv * scale);
    return mod(s.x + s.y, 2.0) == 0.0
        ? vec3(0.2, 0.6, 0.8)
        : vec3(0.9, 0.7, 0.2);
}

void main()
{
    // ── Sample semua texture ──────────────────────────────
    vec4 diffSample = texture(texture_diffuse1,  TexCoords);
    vec4 specSample = texture(texture_specular1, TexCoords);
    vec4 normSample = texture(texture_normal1,   TexCoords);

    vec3 albedo = diffSample.rgb;
    if (dot(albedo, albedo) < 0.001) albedo = vec3(0.8);

    vec3  specColor = specSample.rgb;
    if (dot(specColor, specColor) < 0.001) specColor = vec3(0.5);
    float gloss = specSample.a > 0.01 ? specSample.a : 0.6;

    // Normal — pakai normal map kalau ada, fallback ke vertex normal
    vec3 norm;
    if (dot(normSample.rgb, normSample.rgb) > 0.01) {
        norm = normalize(normSample.rgb * 2.0 - 1.0);
        norm = normalize(Normal + norm * 0.8);
    } else {
        norm = normalize(Normal);
    }

    vec3 result;

    // ══════════════════════════════════════════════════════
    // MODE 1: FINAL RENDER — Realistik
    // ══════════════════════════════════════════════════════
    if (renderMode == 1)
    {
        vec3 viewDir  = normalize(viewPos - FragPos);
        float shininess = mix(32.0, 256.0, gloss);

        // Ambient rendah — biar objek terlihat tiga dimensi
        vec3 lighting = vec3(0.08) * albedo;

        // ── Point lights (dari neon/lab lights) ──────────
        for (int i = 0; i < lightCount && i < MAX_LIGHTS; i++)
        {
            vec3  lDir   = normalize(lightPositions[i] - FragPos);
            float dist   = length(lightPositions[i] - FragPos);
            float atten  = lightIntensities[i] /
                           (1.0 + 0.14*dist + 0.07*dist*dist);

            float diff   = max(dot(norm, lDir), 0.0);
            vec3  halfV  = normalize(lDir + viewDir);
            float spec   = pow(max(dot(norm, halfV), 0.0), shininess);

            lighting += atten * lightColors[i] *
                        (diff * albedo + spec * specColor * 1.5);
        }

        // ── Spotlights per panggung ───────────────────────
        for (int i = 0; i < spotLightCount && i < 4; i++)
        {
            vec3  lDir   = normalize(spotLights[i].position - FragPos);
            float theta  = dot(lDir, normalize(-spotLights[i].direction));
            float eps    = spotLights[i].cutOff - spotLights[i].outerCutOff;
            float si     = clamp((theta - spotLights[i].outerCutOff)/eps, 0.0, 1.0);

            float dist   = length(spotLights[i].position - FragPos);
            float atten  = spotLights[i].intensity /
                           (1.0 + 0.09*dist + 0.032*dist*dist);

            float diff   = max(dot(norm, lDir), 0.0);
            vec3  halfV  = normalize(lDir + viewDir);
            float spec   = pow(max(dot(norm, halfV), 0.0), shininess);

            lighting += si * atten * spotLights[i].color *
                        (diff * albedo + spec * specColor * 2.0);
        }

        result = lighting;

        // Tone mapping Reinhard + gamma correction
        result = result / (result + vec3(1.0));
        result = pow(result, vec3(1.0 / 2.2));
    }

    // ══════════════════════════════════════════════════════
    // MODE 2: BASIC COLOR — Flat texture only
    // ══════════════════════════════════════════════════════
    else if (renderMode == 2)
    {
        result = albedo;
    }

    // ══════════════════════════════════════════════════════
    // MODE 3: MATCAP — Clay/semen
    // ══════════════════════════════════════════════════════
    else if (renderMode == 3)
    {
        float v = (norm.x + norm.y + norm.z) / 6.0 + 0.5;
        result  = vec3(0.6*v, 0.6*v, 0.55*v);
    }

    // ══════════════════════════════════════════════════════
    // MODE 4: UV CHECKER
    // ══════════════════════════════════════════════════════
    else if (renderMode == 4)
    {
        result = uvChecker(TexCoords, 15.0);
    }

    else result = albedo;

    FragColor = vec4(result, diffSample.a);
}