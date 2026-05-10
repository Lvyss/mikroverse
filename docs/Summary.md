# 🧬 MICROVERSE — Resume Proyek & Analisis LearnOpenGL

---

## BAGIAN 1: RESUME PROYEK

### Apa itu Microverse?

Microverse adalah aplikasi **Virtual Laboratory 3D** berbasis OpenGL yang dibuat sebagai Tugas Akhir Grafika Komputer. Konsepnya seperti museum sains interaktif — pengguna bisa berjalan atau terbang di dalam sebuah laboratorium fiksi bergaya sci-fi, lalu mendekati objek biologi (Mitokondria dan Nukleus) yang dipajang di atas panggung display untuk membaca informasi edukatif tentang organel tersebut.

### Stack Teknologi

- **OpenGL 3.3 Core Profile** — rendering engine utama
- **GLFW** — manajemen window dan input keyboard/mouse
- **GLAD** — loader fungsi OpenGL
- **GLM** — matematika vektor dan matriks (transformasi, kamera)
- **Assimp** — loading model 3D dari file `.obj`
- **FreeType** — rendering teks di layar (HUD dan popup)
- **stb_image** — loading file gambar/tekstur
- **Bahasa:** C++ (header-only architecture, semua class di `.h`)

### Fitur yang Sudah Berjalan

**Sistem Kamera & Pergerakan**
Kamera FPS dengan dua mode: Walk Mode (gravitasi aktif, bisa lompat dengan Space, ada collision ke dinding dan platform) dan Fly Mode (bebas 6 arah, sprint dengan Shift). Toggle antara dua mode menggunakan tombol F. Kamera dikontrol mouse untuk look around.

**Ruangan Lab Sci-fi (Lab.h)**
Ruangan box berukuran 28×8×36 unit dibangun manual dengan raw vertex. Lantai bertekstur bata gelap tidak beraturan dengan retakan (procedural di shader), dinding bertekstur panel tech acak, langit-langit bergaya panel dengan channel glow. Terdapat 3 platform sci-fi berbentuk ring (outer ring, inner disk, panel segmen dekoratif) di 3 posisi berbeda. Neon strip lines tersebar di langit-langit dan dinding sebagai dekorasi architectural.

**Pencahayaan**
8 point light neon berwarna biru-putih tersebar di ruangan untuk menerangi dinding dan lantai (room.frag). Setiap model 3D mendapat sistem 4-light studio lighting sendiri (key light, fill light, rim light, bottom light) yang posisinya relatif terhadap center objek (object.frag). Emissive shader terpisah untuk neon lines dan glow ring agar tidak terpengaruh lighting.

**Model 3D**
Dua model diload dengan Assimp: **Mitokondria** (mitokondria.obj) di panggung kiri dan **Nukleus** (nukleus.obj) di panggung kanan. Kedua model auto-scale berdasarkan AABB agar ukurannya konsisten, dan berputar perlahan di atas platform.

**Post-Processing Framebuffer**
Seluruh scene dirender dulu ke FBO (Framebuffer Object), baru ditampilkan ke layar dengan shader post-process. Efek yang diimplementasikan: Bloom (ekstrak pixel terang lalu blur, menyebabkan neon lines bersinar/glowing), Vignette (gelap di pinggir layar), Chromatic Aberration (channel R/G/B sedikit digeser untuk efek "lensa"), dan Contrast boost.

**Sistem Interaksi (InteractionSystem.h)**
Ketika pemain mendekati objek dalam radius tertentu, muncul prompt "Tekan E". Setelah E ditekan, muncul popup edukatif fullscreen dengan konten slide (5 slide per objek). Navigasi slide dengan tombol `<` `>`, tutup dengan Q atau ESC. Popup punya animasi fade-in/fade-out.

**HUD & InfoPanel (InfoPanel.h)**
Panel UI dirender menggunakan custom quad shader (hud.frag) dengan efek rounded corner via SDF (Signed Distance Field). Teks dirender dengan FreeType. Terdapat: prompt interaksi (pill shape dengan tombol E), popup edukatif (layout multi-kolom dengan header, body, fakta menarik, navigasi), dan help popup (muncul 8 detik saat awal, bisa dibuka lagi dengan H).

**Hologram**
Gambar welcome.png ditampilkan sebagai billboard hologram melayang di tengah ruangan. Shader hologram memberi efek scanline, pulse, color shift cyan-ungu, edge glow, dan grid overlay.

**Skybox**
Cubemap solid warna biru sangat gelap sebagai background ruangan (warna cytoplasm).

**MSAA**
Anti-aliasing 4x diaktifkan saat pembuatan window GLFW.

**FPS Counter**
Judul window diupdate tiap detik menampilkan FPS realtime.

---

## BAGIAN 2: ANALISIS LEARNOPENGL — SUDAH DITERAPKAN DI MANA?

---

### 📗 GETTING STARTED

---

**OpenGL**
OpenGL digunakan sebagai seluruh pipeline rendering. Diinisialisasi via `gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)` di main.cpp. Versi yang dipakai adalah OpenGL 3.3 Core Profile, dikonfigurasi dengan `glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3)` dan `GLFW_OPENGL_CORE_PROFILE`. Semua operasi rendering — dari upload vertex data sampai draw call — menggunakan OpenGL API.

---

**Creating a Window**
Di `main.cpp`, window dibuat dengan GLFW. Yang menarik, project ini tidak pakai ukuran hardcode 1280×720 — tapi langsung ambil resolusi monitor dengan `glfwGetPrimaryMonitor()` dan `glfwGetVideoMode()`, lalu buat window fullscreen native. Ini lebih proper dibanding tutorial dasar. Callbacks didaftarkan: `framebuffer_size_callback` untuk resize, `mouse_callback` untuk kamera, `scroll_callback` untuk zoom, `key_callback` untuk input keyboard.

---

**Hello Triangle / VAO & VBO**
Konsep VAO, VBO (dan secara implisit EBO untuk indexed drawing di Mesh) dipakai di mana-mana:

Di `Lab.h`, semua geometri ruangan dan platform dibangun manual dengan `std::vector<float>` berisi vertex data (posisi + normal + UV), lalu diupload ke GPU dengan `glGenVertexArrays`, `glGenBuffers`, `glBufferData`. Fungsi helper `makeVAO()` membuat pola ini berulang untuk setiap bagian geometri (platBodyVAO, platInnerVAO, platPanelVAO, platGlowVAO, ringVAO, neonVAO, skyboxVAO).

Di `Mesh.h`, `setupMesh()` menggunakan VAO + VBO + EBO dengan `glDrawElements` untuk model dari Assimp.

Di `InfoPanel.h`, `setupQuad()` buat quad kecil untuk render UI panel.

Di `HologramText.h`, `setupQuad()` buat billboard quad.

Di `TextRenderer.h`, VAO/VBO dinamis dengan `GL_DYNAMIC_DRAW` untuk upload vertex teks per karakter saat render.

Di `main.cpp`, `renderQuad()` buat fullscreen quad untuk post-processing.

---

**Shaders**
Project ini punya **10 shader program berbeda**, masing-masing untuk kebutuhan berbeda:

- `lighting.vert/frag` — render model dengan Phong + spotlight (dipakai Lab draw room)
- `object.vert/frag` — render model 3D dengan PBR-style (GGX specular, normal map, 4-light studio)
- `room.vert/frag` — render ruangan dengan multi-light dan procedural texture
- `emissive.vert/frag` — render neon lines dan glow ring tanpa terpengaruh lighting
- `hologram.vert/frag` — efek hologram billboard (scanline, pulse, chromatic)
- `hud.vert/frag` — render UI panel dengan rounded corner SDF
- `text.vert/frag` — render glyph teks dari FreeType
- `skybox.vert/frag` — render cubemap background
- `postprocess.vert/frag` — bloom, vignette, chromatic aberration
- `stencil_outline.vert/frag` — outline objek (ada filenya, meski belum aktif dipakai di render loop)

Semua shader dikelola oleh class `Shader.h` yang membaca file, compile, dan link program. Uniform setter tersedia untuk semua tipe data (bool, int, float, vec2, vec3, vec4, mat4).

---

**Textures**
Texture loading dilakukan di dua tempat:

Di `Model.h`, fungsi `TextureFromFile()` menggunakan stb_image untuk load gambar dari disk, upload ke GPU dengan `glTexImage2D`, generate mipmap, dan set parameter wrap/filter. Support channel 1 (GL_RED), 3 (GL_RGB), dan 4 (GL_RGBA).

Di `Lab.h`, dipakai solid color texture 1×1 pixel (`loadSolidColor()`) untuk lantai, dinding, dan platform — ini cara efisien untuk flat color tanpa file gambar eksternal.

Di `HologramText.h`, gambar PNG diload sebagai texture RGBA untuk billboard hologram.

Di `TextRenderer.h`, tiap karakter font FreeType diupload sebagai texture GL_RED 1-channel.

---

**Transformations**
Transformasi menggunakan GLM:

- `glm::translate` — memindahkan model ke posisi platform (`stagePos[i]`)
- `glm::rotate` — memutar model tiap frame (`currentFrame * 0.3f` untuk mitokondria, `* 0.25f` untuk nukleus)
- `glm::scale` — auto-scale model berdasarkan AABB (`mitoAutoScale`, `virusAutoScale`)
- Semua transformasi digabung jadi matrix model dan dikirim ke shader via uniform `model`

Di hologram, ada matrix billboard khusus yang dibangun manual dari vektor right dan up kamera untuk memastikan selalu menghadap pemain.

---

**Coordinate Systems**
Pipeline transformasi lengkap diterapkan:

- **Model Space → World Space:** matrix `model` (translate + rotate + scale)
- **World Space → View Space:** matrix `view` dari `camera.GetViewMatrix()` yang pakai `glm::lookAt`
- **View Space → Clip Space:** matrix `proj` dari `glm::perspective(glm::radians(camera.Zoom), SCR_W/SCR_H, 0.1f, 200.0f)`

Ketiganya dikirim ke setiap shader sebagai uniform `model`, `view`, `projection`. Near plane 0.1f, far plane 200.0f (sesuai ukuran ruangan).

Untuk UI/HUD, dipakai **orthographic projection** `glm::ortho(0, SCR_W, 0, SCR_H)` agar koordinat pixel langsung dipakai tanpa depth effect.

---

**Camera**
Implementasi kamera FPS custom ada di `Camera.h`:

- Vektor `Front`, `Right`, `Up` dihitung dari Yaw dan Pitch via `updateCameraVectors()`
- `ProcessMouseMovement()` mengupdate Yaw dan Pitch dari delta mouse, dengan pitch clamp ±89°
- `ProcessMouseScroll()` mengubah field of view (Zoom)
- `GetViewMatrix()` mengembalikan `glm::lookAt(Position, Position + Front, Up)`
- Mode Walk: gerak hanya di bidang horizontal (Y diabaikan dari arah Front), ada gravitasi, lompat, dan collision
- Mode Fly: gerak bebas 6 arah termasuk Up/Down, sprint dengan shift
- `UpdatePhysics()` menangani gravitasi (18 unit/s²), terminal velocity, collision platform, collision lantai/langit-langit

---

### 💡 LIGHTING

---

**Colors**
Konsep warna cahaya terpisah dari warna objek diterapkan di seluruh sistem. Di `room.frag`, neon light berwarna `vec3(0.55, 0.75, 1.0)` (biru) dikalikan dengan albedo objek. Di `object.frag`, 4 light source menggunakan warna putih `vec3(1.0)` agar tidak mengubah warna asli model. Di `emissive.frag`, `emissiveColor * emissiveStrength` menghasilkan warna neon murni. Warna glow ring per platform berbeda (biru untuk mitokondria/nukleus, emas untuk platform ketiga).

---

**Basic Lighting (Phong)**
Model Phong diimplementasikan di `lighting.frag` dan `room.frag`:

- **Ambient:** `vec3(0.08) * albedo` — cahaya dasar sangat gelap biar objek terlihat 3D
- **Diffuse:** `max(dot(norm, lightDir), 0.0) * albedo` — bergantung sudut normal terhadap arah cahaya
- **Specular:** `pow(max(dot(norm, halfVec), 0.0), shininess) * specColor` — highlight mengkilat, pakai Half Vector (Blinn-Phong)

Di `object.frag`, dipakai model PBR yang lebih advanced: **GGX Distribution** (`distributionGGX`), **Geometry Smith** (`geometrySmith`), dan **Fresnel Schlick** untuk specular yang lebih realistis dibanding Phong klasik.

---

**Materials**
Properties material dipakai di berbagai tempat:

Di `object.frag`, `roughness` dan `metallic` dibaca dari channel G dan B texture specular (`specSample.g`, `specSample.b`). Roughness mengontrol lebar highlight, metallic mengontrol seberapa "logam" permukaan terlihat. Jika texture tidak ada, fallback ke nilai default (roughness 0.45, metallic 0.05).

Di `room.frag`, setiap surface type punya nilai `shininess` dan `specStr` berbeda: lantai (shininess 25, specStr 0.08 — matt), dinding (shininess 50, specStr 0.25 — semi-glossy), langit-langit (shininess 12, specStr 0.06 — sangat matt).

---

**Lighting Maps**
Texture diffuse dan texture specular dipakai sebagai lighting maps:

Di `lighting.frag`: `texture_diffuse1` sebagai albedo/warna dasar, `texture_specular1` untuk warna dan glossiness specular, `texture_normal1` untuk normal map.

Di `object.frag`: sama, tapi specular map juga menyimpan roughness (channel G) dan metallic (channel B) — ini format PBR standard.

Di `Mesh.h` dan `Model.h`, texture di-load per-type: `texture_diffuse`, `texture_specular`, `texture_normal`, `texture_height`. Nama uniform dikirim ke shader agar shader bisa sample texture yang benar.

---

**Light Casters**
Tiga tipe light caster diimplementasikan:

**Directional Light** — tidak secara eksplisit, tapi ambient global berfungsi mirip.

**Point Light** — di `room.frag`, 8 point light neon dengan atenuasi `lightRadius / (1 + 0.14*dist + 0.07*dist²)`. Di `Lab.h`, `pointLightPositions` dan `pointLightColors` didefinisikan. Di `object.frag`, 4 point light studio dengan atenuasi `lightStrengths[i] / (1 + 0.09*dist + 0.032*dist²)`.

**Spotlight** — di `Lab.h`, 3 spotlight tepat di atas tiap platform, diarahkan ke bawah `(0,-1,0)` dengan cutoff angle `cos(18°)` dan outer cutoff `cos(28°)`. Di `lighting.frag`, ada kalkulasi spotlight lengkap dengan `smoothstep(outerCutOff, cutOff, theta)` untuk soft edge.

---

**Multiple Lights**
`room.frag` menangani **8 lampu sekaligus** dalam satu loop `for (int i = 0; i < numLights)`. `lighting.frag` menangani point lights (4 max) **dan** spotlights (4 max) dalam dua loop terpisah. `object.frag` menangani 4 studio light dalam satu loop. Ini menunjukkan implementasi multiple light sources yang proper.

---

### 📦 MODEL LOADING

---

**Assimp**
Di `Model.h`, Assimp dipakai untuk parse file 3D:

```
Assimp::Importer imp;
const aiScene* scene = imp.ReadFile(path,
    aiProcess_Triangulate | aiProcess_GenSmoothNormals |
    aiProcess_FlipUVs     | aiProcess_CalcTangentSpace);
```

Flag yang dipakai: `Triangulate` (konversi semua polygon ke segitiga), `GenSmoothNormals` (generate normal jika tidak ada), `FlipUVs` (flip koordinat UV agar sesuai OpenGL), `CalcTangentSpace` (hitung tangent dan bitangent untuk normal mapping). Dua model diload: `models/mitokondria/mitokondria.obj` dan `models/nukleus/nukleus.obj`.

---

**Mesh**
Class `Mesh` di `Mesh.h` persis mengikuti struktur LearnOpenGL:

- Struct `Vertex` berisi Position, Normal, TexCoords, Tangent, Bitangent
- Struct `Texture` berisi id, type, path
- `setupMesh()` membuat VAO + VBO + EBO dan set vertex attribute pointers untuk semua 5 attribute (location 0-4)
- `Draw()` bind semua texture ke unit yang sesuai, kirim nama uniform ke shader, lalu `glDrawElements`
- Cache texture di `textures_loaded` agar texture yang sama tidak diload dua kali

---

**Model**
Class `Model` di `Model.h` menggunakan Assimp untuk traverse node tree secara rekursif:

- `loadModel()` buka file dengan Assimp
- `processNode()` rekursif traverse semua node di scene graph
- `processMesh()` konversi `aiMesh` ke class `Mesh` sendiri: ekstrak vertex positions, normals, UVs, tangents, bitangents; ekstrak indices dari faces; load texture dari material (`aiTextureType_DIFFUSE`, `SPECULAR`, `NORMALS`, `HEIGHT`)
- `Draw()` tinggal panggil `Draw()` di semua mesh yang tersimpan

Di `main.cpp`, AABB (Axis-Aligned Bounding Box) dihitung secara manual dengan iterasi semua vertex di semua mesh untuk auto-scale model agar proporsional di scene.

---

### ✅ RINGKASAN CAKUPAN

| Bab | Sub-topik | Status |
|-----|-----------|--------|
| Getting Started | OpenGL, Window, VAO/VBO, Shaders | ✅ Lengkap |
| Getting Started | Textures, Transformations | ✅ Lengkap |
| Getting Started | Coordinate Systems, Camera | ✅ Lengkap |
| Lighting | Colors, Basic Lighting (Phong) | ✅ Lengkap |
| Lighting | Materials, Lighting Maps | ✅ Lengkap |
| Lighting | Light Casters (Point + Spot) | ✅ Lengkap |
| Lighting | Multiple Lights | ✅ Lengkap |
| Model Loading | Assimp, Mesh, Model | ✅ Lengkap |
| Advanced OpenGL | Framebuffers (post-process) | ✅ Ada |
| Advanced OpenGL | Cubemaps (skybox) | ✅ Ada |
| Advanced OpenGL | Anti-aliasing MSAA 4x | ✅ Ada |
| Advanced OpenGL | Depth Testing | ✅ Ada (implisit) |
| Advanced OpenGL | Stencil Buffer | ⚠️ Shader ada, belum dipakai aktif |
| Advanced OpenGL | Blending | ⚠️ Dipakai untuk hologram & UI, bukan membran sel |
| Advanced OpenGL | Instancing | ❌ Belum diimplementasi |
| Advanced Lighting | Normal Mapping | ✅ Ada di object.frag (TBN matrix) |
| In Practice | Text Rendering FreeType | ✅ Lengkap |

---

Semua topik **Getting Started**, **Lighting**, dan **Model Loading** tercakup penuh dan bisa didemonstrasikan secara langsung di project ini. Untuk Advanced OpenGL, sebagian besar sudah ada (framebuffer, cubemap, MSAA, blending) tapi beberapa seperti instancing dan stencil outline belum aktif di render loop final.
