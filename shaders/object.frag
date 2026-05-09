#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
} fs_in;

uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform sampler2D texture_normal1;
uniform vec3 viewPos;
uniform vec3 objectCenter;

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH2 = max(dot(N, H), 0.0) * max(dot(N, H), 0.0);
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159265 * denom * denom);
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return geometrySchlickGGX(max(dot(N,V),0.0), roughness)
         * geometrySchlickGGX(max(dot(N,L),0.0), roughness);
}

void main()
{
    // ── Sample textures — warna ASLI tanpa modifikasi ─────
    vec3 albedo = texture(texture_diffuse1,  fs_in.TexCoords).rgb;
    vec4 specSample = texture(texture_specular1, fs_in.TexCoords);
    vec4 normSample = texture(texture_normal1,   fs_in.TexCoords);

    // Fallback kalau texture tidak ke-load
    if (dot(albedo, albedo) < 0.001) albedo = vec3(0.7);

    // Roughness & metallic
    float roughness = specSample.g > 0.01 ? specSample.g : 0.45;
    float metallic  = specSample.b > 0.01 ? specSample.b : 0.05;
    if (dot(specSample.rgb, specSample.rgb) < 0.001) {
        roughness = 0.45;
        metallic  = 0.05;
    }

    // Normal map
    vec3 N;
    if (dot(normSample.rgb, normSample.rgb) > 0.01)
        N = normalize(fs_in.TBN * normalize(normSample.rgb * 2.0 - 1.0));
    else
        N = normalize(fs_in.Normal);

    vec3 V = normalize(viewPos - fs_in.FragPos);

    // ── Light setup ───────────────────────────────────────
    vec3  lightPositions[4];
    float lightStrengths[4];

    // KEY — kanan atas depan (paling kuat)
    lightPositions[0] = objectCenter + vec3( 4.0,  5.0,  3.0);
    lightStrengths[0] = 2.2;

    // FILL — kiri depan (lembut)
    lightPositions[1] = objectCenter + vec3(-3.5,  1.5,  3.0);
    lightStrengths[1] = 0.6;

    // RIM — belakang atas (edge highlight)
    lightPositions[2] = objectCenter + vec3(-2.0,  3.5, -4.0);
    lightStrengths[2] = 0.7;

    // BOTTOM — bawah (fill sangat lembut)
    lightPositions[3] = objectCenter + vec3( 0.0, -3.0,  1.0);
    lightStrengths[3] = 0.25;

    // ── Hitung lighting ───────────────────────────────────
    // Ambient — warna asli, tidak dimodifikasi
    float ambientStr = 0.35;
    vec3  result     = ambientStr * albedo;

    for (int i = 0; i < 4; i++)
    {
        vec3  L    = normalize(lightPositions[i] - fs_in.FragPos);
        vec3  H    = normalize(V + L);
        float dist = length(lightPositions[i] - fs_in.FragPos);
        float atten = lightStrengths[i] /
                      (1.0 + 0.09*dist + 0.032*dist*dist);

        float NdotL = max(dot(N, L), 0.0);

        // Diffuse — HANYA menggunakan warna albedo asli
        // Cahaya putih (vec3(1.0)) tidak mengubah warna
        vec3 diffuse = albedo * NdotL;

        // Specular — putih netral, tidak mewarnai
        float NDF  = distributionGGX(N, H, roughness);
        float G    = geometrySmith(N, V, L, roughness);
        // Fresnel netral — tidak mewarnai specular
        vec3  F    = vec3(0.04) + (1.0 - 0.04) *
                     pow(clamp(1.0 - max(dot(H,V),0.0), 0.0, 1.0), 5.0);
        float denom = 4.0 * max(dot(N,V),0.0) * NdotL + 0.0001;
        vec3  spec  = (NDF * G * F / denom) * 0.3; // specular putih netral

        result += atten * (diffuse + spec);
    }

    // ── TIDAK ada tone mapping / gamma yang mengubah warna ─
    // Clamp saja biar tidak overexposed
    result = clamp(result, 0.0, 1.0);

    FragColor = vec4(result, 1.0);
}