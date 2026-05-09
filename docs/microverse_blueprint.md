# 🧬 Microverse — Blueprint Proyek Tugas Akhir Grafika Komputer

> **Tema:** Virtual Laboratory 3D untuk Visualisasi Struktur Sel dan Interaksi Virus pada Manusia
> **Teknologi:** OpenGL + GLFW + GLAD + Assimp + FreeType
> **Target:** Semua materi LearnOpenGL tercakup
> **Deadline:** 2 Minggu

---

## 📋 Daftar Isi

1. [Gambaran Umum Aplikasi](#1-gambaran-umum-aplikasi)
2. [Desain Laboratorium (Denah)](#2-desain-laboratorium-denah)
3. [Fitur & Mapping LearnOpenGL](#3-fitur--mapping-learnopengl)
4. [Roadmap 2 Minggu](#4-roadmap-2-minggu)
5. [Struktur Folder Project](#5-struktur-folder-project)
6. [Daftar Model 3D (Sketchfab)](#6-daftar-model-3d-sketchfab)
7. [Detail Implementasi Per Fitur](#7-detail-implementasi-per-fitur)
8. [Kontrol Aplikasi](#8-kontrol-aplikasi)
9. [Checklist Presentasi](#9-checklist-presentasi)

---

## 1. Gambaran Umum Aplikasi

**Microverse** adalah aplikasi virtual laboratory 3D berbasis OpenGL yang memungkinkan pengguna menjelajahi dunia mikroskopis sel manusia dan interaksi virus secara imersif. Konsepnya seperti museum sains interaktif — pengguna berjalan (atau terbang) di dalam laboratorium dan mengamati setiap objek biologis yang dipajang di atas panggung display.

### Konsep Inti
- **Mode Eksplorasi:** Walking (gravitasi + collision) dan Flying (noclip bebas), toggle dengan tombol `F`
- **Layout:** 1 ruangan besar bergaya museum dengan 4 panggung display di sisi kiri dan kanan
- **Skala:** Mikroskopis — seolah pengguna mengecil dan masuk ke dalam tubuh manusia
- **Objek:** Model 3D diunduh dari Sketchfab (.glb / .obj), di-load menggunakan Assimp

---

## 2. Desain Laboratorium (Denah)

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│  ┌──────────────┐   Lorong Utama   ┌──────────────┐    │
│  │  PANGGUNG 1  │                  │  PANGGUNG 2  │    │
│  │  Sel Normal  │ ←── walk/fly ──→ │    Virus     │    │
│  │  (organel)   │                  │  (spike prot)│    │
│  └──────────────┘                  └──────────────┘    │
│                                                         │
│              [ Info Panel HUD + Minimap ]               │
│                                                         │
│  ┌──────────────┐                  ┌──────────────┐    │
│  │  PANGGUNG 3  │                  │  PANGGUNG 4  │    │
│  │   Infeksi    │                  │ Sistem Imun  │    │
│  │  (simulasi)  │                  │(sel T, anti) │    │
│  └──────────────┘                  └──────────────┘    │
│                                                         │
│                   [ Pintu Masuk ]                       │
└─────────────────────────────────────────────────────────┘
```

### Deskripsi Tiap Area

| Area | Isi | Efek Visual |
|------|-----|-------------|
| **Panggung 1 — Sel Normal** | Model sel manusia lengkap dengan nukleus, mitokondria, ribosom, membran | Spotlight biru, partikel protein melayang, label FreeType |
| **Panggung 2 — Virus** | Model coronavirus / virus influenza dengan spike protein | Spotlight merah, animasi rotasi lambat, stencil outline |
| **Panggung 3 — Simulasi Infeksi** | Virus mendekati dan menempel ke membran sel | Blending transparansi, instancing banyak virus, warna dramatis |
| **Panggung 4 — Sistem Imun** | Sel T, antibodi, fagosit menyerang virus | Particle system, multiple light sources |
| **Lorong Utama** | Area bebas jelajah pengguna | Cubemap skybox cytoplasm, ambient lighting |

---

## 3. Fitur & Mapping LearnOpenGL

| # | Fitur Aplikasi | Topik LearnOpenGL | Chapter |
|---|---------------|-------------------|---------|
| 1 | Ruangan 3D + kamera FPS | VAO, VBO, EBO, Shaders, Coordinate Systems, Camera | Getting Started |
| 2 | Pencahayaan per panggung | Phong Lighting, Materials, Light Maps, Light Casters | Lighting |
| 3 | Load model sel & virus | Model Loading (Assimp), Mesh class | Model Loading |
| 4 | Transparansi membran sel | Blending, Depth Testing | Advanced OpenGL |
| 5 | Outline objek aktif | Stencil Buffer | Advanced OpenGL |
| 6 | Cubemap cytoplasm | Cubemaps, Face Culling | Advanced OpenGL |
| 7 | Efek lensa mikroskop | Framebuffers, Kernel Effects | Advanced OpenGL |
| 8 | Banyak virus (instancing) | Instancing | Advanced OpenGL |
| 9 | Partikel protein melayang | Instancing + Geometry Shader | Advanced OpenGL |
| 10 | Label nama organel | Text Rendering (FreeType) | In Practice |
| 11 | Permukaan membran kasar | Normal Mapping | Advanced Lighting |
| 12 | Bayangan organel | Shadow Mapping | Advanced Lighting |
| 13 | Debug garis normal | Geometry Shader | Advanced OpenGL |
| 14 | Anti-aliasing | MSAA | Advanced OpenGL |
| 15 | Walk + Fly toggle | Camera + Physics custom | Custom |

---

## 4. Roadmap 2 Minggu

### Fase 1 — Hari 1–3: Fondasi OpenGL

**Target:** Ruangan kosong bisa dijelajahi, dua mode kamera jalan.

- [ ] Setup project: CMake / Visual Studio, link GLFW + GLAD + GLM
- [ ] Buat window + render loop dasar
- [ ] Implementasi VAO/VBO untuk box primitif (lantai, dinding, langit-langit)
- [ ] Shader vertex + fragment dasar (warna flat)
- [ ] Kamera FPS (mouse look, WASD)
- [ ] **Walk mode:** simulasi gravitasi sederhana, collision AABB dinding
- [ ] **Fly mode:** noclip, gerak bebas 6 arah
- [ ] Toggle Walk/Fly dengan tombol `F`
- [ ] Buat 4 platform/pedestal (box) sebagai placeholder panggung

**Output Fase 1:** Bisa jalan-jalan di lab kosong, kamera mulus, dua mode bekerja.

---

### Fase 2 — Hari 4–7: Model Loading + Lighting

**Target:** 4 panggung terisi model 3D dengan pencahayaan layak.

- [ ] Download semua model dari Sketchfab (lakukan di hari 4 pagi)
- [ ] Konversi model ke format .obj atau .glb bila perlu (pakai Blender free)
- [ ] Integrasi Assimp ke project
- [ ] Buat class `Model` dan `Mesh` mengikuti struktur LearnOpenGL
- [ ] Load model sel normal → taruh di Panggung 1
- [ ] Load model virus → taruh di Panggung 2
- [ ] Load/buat scene infeksi → Panggung 3
- [ ] Load model sel imun → Panggung 4
- [ ] Implementasi Phong lighting (ambient + diffuse + specular)
- [ ] Spotlight di atas tiap panggung
- [ ] Ambient light rendah untuk suasana lab gelap

**Output Fase 2:** 4 panggung berisi model 3D dengan pencahayaan dramatis.

> ⚠️ **Perhatian:** Fase ini paling berat. Jika ada masalah model, alokasikan hari ke-7 sebagai buffer.

---

### Fase 3 — Hari 8–11: Efek Visual + Interaksi

**Target:** Atmosfer laboratorium mikro yang imersif.

- [ ] **Cubemap skybox** — tekstur cytoplasm / luar angkasa sel sebagai background
- [ ] **Blending** — membran sel semi-transparan (alpha < 1.0)
- [ ] **Stencil buffer** — outline glow pada objek yang sedang dilihat
- [ ] **Framebuffer post-processing** — efek chromatic aberration / blur ringan lensa mikroskop
- [ ] **Instancing** — 50–100 partikel protein/ribosom kecil melayang di lorong
- [ ] **Geometry Shader** — visualisasi garis normal (mode debug, tombol `N`)
- [ ] **FreeType text rendering** — label nama melayang di tiap panggung
- [ ] **Normal Mapping** — tekstur permukaan kasar pada membran sel

**Output Fase 3:** Lab terasa hidup, ada partikel, efek, dan label informatif.

---

### Fase 4 — Hari 12–14: Polish + Presentasi

**Target:** Aplikasi siap demo dan presentasi.

- [ ] **Shadow Mapping** sederhana (directional light dari atas) — skip jika waktu kurang
- [ ] **MSAA** anti-aliasing (set `samples = 4` saat buat window)
- [ ] **HUD overlay:** nama area/panggung yang sedang dekat, mode aktif (Walk/Fly), FPS counter
- [ ] **Minimap 2D** sederhana di pojok layar (optional, tapi keren untuk presentasi)
- [ ] Bug fixing menyeluruh: collision edge case, z-fighting, texture loading error
- [ ] Optimasi: frustum culling sederhana, batasi draw call
- [ ] Rekam video demo 3–5 menit untuk presentasi
- [ ] Tulis laporan / dokumentasi singkat

**Output Fase 4:** Aplikasi polish, demo video siap, laporan selesai.

---

## 5. Struktur Folder Project

```
Microverse/
├── CMakeLists.txt
├── main.cpp
│
├── src/
│   ├── core/
│   │   ├── Shader.h / Shader.cpp
│   │   ├── Camera.h / Camera.cpp        ← walk + fly mode di sini
│   │   ├── Window.h / Window.cpp
│   │   └── InputHandler.h
│   │
│   ├── rendering/
│   │   ├── Mesh.h / Mesh.cpp
│   │   ├── Model.h / Model.cpp          ← Assimp loader
│   │   ├── Framebuffer.h / Framebuffer.cpp
│   │   ├── Skybox.h / Skybox.cpp
│   │   ├── TextRenderer.h / TextRenderer.cpp  ← FreeType
│   │   └── ParticleSystem.h / ParticleSystem.cpp
│   │
│   ├── scene/
│   │   ├── Lab.h / Lab.cpp              ← layout ruangan
│   │   ├── Stage.h / Stage.cpp          ← tiap panggung
│   │   └── HUD.h / HUD.cpp
│   │
│   └── physics/
│       └── Collision.h / Collision.cpp  ← AABB walk mode
│
├── shaders/
│   ├── basic.vert / basic.frag
│   ├── lighting.vert / lighting.frag
│   ├── skybox.vert / skybox.frag
│   ├── postprocess.vert / postprocess.frag
│   ├── text.vert / text.frag
│   ├── particle.vert / particle.frag / particle.geom
│   ├── normals.vert / normals.frag / normals.geom   ← debug
│   └── shadow.vert / shadow.frag
│
├── models/
│   ├── cell_normal/         ← dari Sketchfab
│   ├── virus/               ← dari Sketchfab
│   ├── immune_cell/         ← dari Sketchfab
│   └── lab_props/           ← pedestal, meja, dll
│
├── textures/
│   ├── skybox/              ← 6 face cubemap
│   ├── floor.png
│   ├── wall.png
│   └── particle.png
│
├── fonts/
│   └── arial.ttf            ← atau font bebas lainnya
│
└── include/
    ├── glad/
    ├── GLFW/
    ├── glm/
    ├── assimp/
    └── freetype/
```

---

## 6. Daftar Model 3D (Sketchfab)

Download semua model di **hari pertama** sebelum mulai coding.

| Kebutuhan | Keyword Pencarian | Format Target | Prioritas |
|-----------|-------------------|---------------|-----------|
| Sel manusia lengkap | `human cell organelles 3d` | .glb / .obj | Wajib |
| Nukleus saja | `cell nucleus 3d` | .glb / .obj | Wajib |
| Coronavirus / virus | `coronavirus 3d`, `sars cov2` | .glb / .obj | Wajib |
| Sel T / limfosit | `t cell lymphocyte 3d` | .glb / .obj | Wajib |
| Antibodi | `antibody molecule 3d` | .glb / .obj | Opsional |
| Mitokondria | `mitochondria 3d` | .glb / .obj | Opsional |

**Filter Sketchfab yang harus dicentang:**
- ✅ Free
- ✅ Downloadable
- Format: pilih **GLTF/GLB** atau **OBJ** saat download

**Konversi model (jika format tidak didukung):**
```
Buka Blender → File → Import → [format apapun]
File → Export → Wavefront (.obj) atau glTF 2.0 (.glb)
```

---

## 7. Detail Implementasi Per Fitur

### 7.1 Walk Mode (Gravitasi + Collision)

```cpp
// Di Camera.cpp
void Camera::updateWalkMode(float deltaTime) {
    // Gravitasi
    velocity.y -= 9.8f * deltaTime;
    position += velocity * deltaTime;
    
    // Collision lantai
    if (position.y < FLOOR_HEIGHT + PLAYER_HEIGHT) {
        position.y = FLOOR_HEIGHT + PLAYER_HEIGHT;
        velocity.y = 0.0f;
    }
    
    // Collision dinding (AABB sederhana)
    position.x = glm::clamp(position.x, LAB_MIN_X + 0.5f, LAB_MAX_X - 0.5f);
    position.z = glm::clamp(position.z, LAB_MIN_Z + 0.5f, LAB_MAX_Z - 0.5f);
}
```

### 7.2 Toggle Walk/Fly

```cpp
// Di InputHandler
if (key == GLFW_KEY_F && action == GLFW_PRESS) {
    camera.isFlyMode = !camera.isFlyMode;
    if (!camera.isFlyMode) camera.velocity = glm::vec3(0.0f); // reset velocity
}
```

### 7.3 Spotlight Per Panggung

```cpp
// Tiap panggung punya spotlight dari atas
struct SpotLight {
    glm::vec3 position;   // tepat di atas model
    glm::vec3 direction;  // ke bawah (0, -1, 0)
    float cutOff;         // glm::cos(glm::radians(15.0f))
    float outerCutOff;    // glm::cos(glm::radians(25.0f))
    glm::vec3 ambient, diffuse, specular;
};
```

### 7.4 Instancing Partikel

```cpp
// Buat 100 partikel kecil
glm::mat4 particleMatrices[100];
for (int i = 0; i < 100; i++) {
    glm::mat4 model = glm::mat4(1.0f);
    // Posisi acak di sekitar lorong
    model = glm::translate(model, glm::vec3(
        randomFloat(-5.0f, 5.0f),
        randomFloat(0.5f, 3.0f),
        randomFloat(-10.0f, 10.0f)
    ));
    model = glm::scale(model, glm::vec3(0.05f)); // partikel kecil
    particleMatrices[i] = model;
}
// Upload ke instance VBO, render sekali dengan glDrawArraysInstanced
```

### 7.5 Post-Processing Framebuffer

```cpp
// Render scene ke FBO dulu
glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    renderScene(); // semua object
glBindFramebuffer(GL_FRAMEBUFFER, 0);

// Lalu render quad dengan shader efek
postProcessShader.use();
postProcessShader.setInt("effect", currentEffect); // 0=normal, 1=blur, 2=aberration
renderQuad(); // fullscreen quad
```

Fragment shader efek lensa mikroskop:
```glsl
// postprocess.frag — chromatic aberration ringan
vec2 offset = vec2(0.003, 0.003);
vec3 col;
col.r = texture(screenTexture, TexCoords + offset).r;
col.g = texture(screenTexture, TexCoords).g;
col.b = texture(screenTexture, TexCoords - offset).b;
FragColor = vec4(col, 1.0);
```

### 7.6 Cubemap Skybox Cytoplasm

```cpp
// Load 6 gambar untuk faces cubemap
vector<string> faces = {
    "textures/skybox/right.jpg",   // +X
    "textures/skybox/left.jpg",    // -X
    "textures/skybox/top.jpg",     // +Y
    "textures/skybox/bottom.jpg",  // -Y
    "textures/skybox/front.jpg",   // +Z
    "textures/skybox/back.jpg"     // -Z
};
// Gunakan tekstur warna teal/biru dengan noise organik
// Bisa buat sendiri di GIMP / Photoshop dengan filter cloud
```

### 7.7 Stencil Outline Objek Aktif

```cpp
// Pass 1: render objek normal, tulis ke stencil
glEnable(GL_STENCIL_TEST);
glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
glStencilFunc(GL_ALWAYS, 1, 0xFF);
renderModel(activeModel, normalShader);

// Pass 2: render sedikit lebih besar, hanya di mana stencil != 1
glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
glStencilMask(0x00);
glDisable(GL_DEPTH_TEST);
renderModel(activeModel, outlineShader); // shader warna solid kuning
glEnable(GL_DEPTH_TEST);
glStencilMask(0xFF);
```

---

## 8. Kontrol Aplikasi

| Tombol | Aksi |
|--------|------|
| `W A S D` | Gerak maju/mundur/kiri/kanan |
| `Mouse` | Lihat kanan/kiri/atas/bawah |
| `Space` | Lompat (walk mode) / naik (fly mode) |
| `Shift` | Turun (fly mode) |
| `F` | Toggle Walk ↔ Fly |
| `E` | Interact / lihat info objek terdekat |
| `N` | Toggle debug normal lines |
| `P` | Toggle post-processing efek |
| `Tab` | Tampilkan/sembunyikan minimap |
| `ESC` | Keluar aplikasi |

---

## 9. Checklist Presentasi

### Materi LearnOpenGL yang Harus Dicakup

**Getting Started**
- [ ] VAO, VBO, EBO digunakan untuk render ruangan
- [ ] Shader program (vertex + fragment) custom
- [ ] Texture loading (tekstur lantai, dinding, model)
- [ ] Transformasi: translate, rotate, scale (tiap objek panggung)
- [ ] Coordinate Systems: model → world → view → projection
- [ ] Camera FPS dengan mouse look

**Lighting**
- [ ] Phong lighting model (ambient + diffuse + specular)
- [ ] Material properties per objek
- [ ] Light Maps (diffuse + specular map)
- [ ] Multiple light types: Point Light (ambient lab) + Spotlight (tiap panggung)

**Model Loading**
- [ ] Assimp berhasil load minimal 2 model eksternal
- [ ] Mesh class dan Model class terstruktur
- [ ] Texture dari model terbaca dan tampil benar

**Advanced OpenGL**
- [ ] Depth Testing (semua objek ter-render benar, tidak z-fighting)
- [ ] Stencil Buffer (outline objek aktif)
- [ ] Blending (membran sel transparan)
- [ ] Face Culling (aktif untuk performa)
- [ ] Framebuffers (post-processing efek)
- [ ] Cubemaps (skybox cytoplasm)
- [ ] Geometry Shader (debug normal, atau partikel)
- [ ] Instancing (partikel protein / banyak virus)
- [ ] Anti-aliasing MSAA

**Advanced Lighting**
- [ ] Normal Mapping (permukaan membran kasar)
- [ ] Shadow Mapping (minimal shadow dari 1 spotlight) — opsional jika waktu kurang

**In Practice**
- [ ] Text Rendering FreeType (label nama panggung/organel)

### Hal yang Harus Bisa Dijelaskan Saat Presentasi

- [ ] Kenapa pakai VAO/VBO dan bagaimana cara kerjanya
- [ ] Perbedaan walk mode vs fly mode (implementasi)
- [ ] Cara Assimp parse file 3D menjadi Mesh
- [ ] Cara kerja Phong lighting di shader
- [ ] Kenapa stencil buffer dipakai untuk outline
- [ ] Cara cubemap di-render sebagai background
- [ ] Cara instancing menghemat draw call
- [ ] Alur framebuffer post-processing

---

## 📌 Tips & Peringatan

> **Download model duluan!** Jangan tunggu hari ke-4. Cari dan simpan semua model di hari pertama karena kadang susah nemuin yang free dan berkualitas bagus.

> **Jika kehabisan waktu**, prioritaskan: Kamera + Model Loading + Lighting + Cubemap + Framebuffer. Itu sudah cukup untuk dapat nilai baik. Shadow dan Normal Mapping bisa jadi bonus.

> **Format model paling aman:** `.obj` + `.mtl` karena paling mudah debug jika ada masalah. GLB lebih modern tapi kadang material tidak terbaca sempurna di Assimp versi lama.

> **Jangan lupa deltaTime** di semua kalkulasi gerak agar kecepatan konsisten di semua komputer:
> ```cpp
> float deltaTime = currentFrame - lastFrame;
> lastFrame = currentFrame;
> ```

---

*Blueprint ini dibuat untuk proyek Tugas Akhir Grafika Komputer — Microverse Virtual Laboratory 3D*
