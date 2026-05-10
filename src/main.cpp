// main.cpp
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <algorithm>
#include <sstream>

#include "core/Camera.h"
#include "core/Shader.h"
#include "rendering/Model.h"
#include "scene/Lab.h"
#include "rendering/HologramText.h"
#include "scene/InteractionSystem.h"
#include "scene/InfoPanel.h"
#include "rendering/TextRenderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// ── Settings ──────────────────────────────────────────────


// ── Globals ───────────────────────────────────────────────
Camera camera(glm::vec3(0.0f, FLOOR_HEIGHT + PLAYER_HEIGHT, 14.0f));
float lastX = 960.0f, lastY = 540.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;
int postEffect = 4;

// Pointer global ke interactSys agar bisa diakses di key_callback
InteractionSystem* g_interactSys = nullptr;
InfoPanel* g_infoPanel = nullptr;   // ← TAMBAH
// ── Callbacks ─────────────────────────────────────────────
void framebuffer_size_callback(GLFWwindow*, int w, int h) { glViewport(0, 0, w, h); }

void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (firstMouse) { lastX = xpos; lastY = ypos; firstMouse = false; }
    camera.ProcessMouseMovement(xpos - lastX, lastY - ypos);
    lastX = xpos; lastY = ypos;
}

void scroll_callback(GLFWwindow*, double, double yoff) { camera.ProcessMouseScroll(yoff); }

void key_callback(GLFWwindow* win, int key, int, int action, int)
{
    if (action != GLFW_PRESS) return;

    if (g_interactSys && g_interactSys->popupOpen) {
        if (key == GLFW_KEY_RIGHT)   { g_interactSys->nextSlide();  return; }
        if (key == GLFW_KEY_LEFT)    { g_interactSys->prevSlide();  return; }
        if (key == GLFW_KEY_Q || key == GLFW_KEY_ESCAPE) {
            g_interactSys->closePopup();
            return;
        }
    }

    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(win, true);
        return;
    }
    if (key == GLFW_KEY_SPACE)
        camera.Jump();
    if (key == GLFW_KEY_P) {
        postEffect = (postEffect + 1) % 4;
        std::cout << "Post Effect: " << postEffect << "\n";
    }
    if (key == GLFW_KEY_E && g_interactSys)
        g_interactSys->onInteract();

    // ← TAMBAH INI
    if (key == GLFW_KEY_H && g_infoPanel)
        g_infoPanel->showHelpPopup();
}
void processInput(GLFWwindow* win)
{
    if (g_interactSys && g_interactSys->popupOpen) return;

    // ========== TOGGLE FLY/WALK MODE (EDGE DETECTION) ==========
    static bool lastFState = false;                    // simpan state F sebelumnya
    bool currentFState = (glfwGetKey(win, GLFW_KEY_F) == GLFW_PRESS);  // cek apakah F ditekan sekarang
    
    if (currentFState && !lastFState) {                // EDGE: dari ga ditekan jadi ditekan
        camera.ToggleMode();                           // toggle mode SEKALI
        std::cout << "Mode: " << (camera.isFlyMode ? "FLY (melayang)" : "WALK (gravitasi)") << "\n";
    }
    lastFState = currentFState;                        // update state untuk frame berikutnya
    // ==========================================================

    // Sprint hanya di fly mode (shift)
    bool sprint = (glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
    camera.SetSprinting(sprint);

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD,  deltaTime);
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(win, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT,     deltaTime);
    if (glfwGetKey(win, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT,    deltaTime);

    // Up/Down hanya di fly mode
    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camera.ProcessKeyboard(UP,   deltaTime);
    if (glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS && camera.isFlyMode)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

// ── Fullscreen Quad ────────────────────────────────────────
unsigned int quadVAO = 0, quadVBO;
void renderQuad()
{
    if (quadVAO == 0) {
        float verts[] = {
            -1, 1, 0, 1,  -1,-1, 0, 0,   1,-1, 1, 0,
            -1, 1, 0, 1,   1,-1, 1, 0,   1, 1, 1, 1
        };
        glGenVertexArrays(1, &quadVAO); glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ── Main ──────────────────────────────────────────────────
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

GLFWmonitor* monitor    = glfwGetPrimaryMonitor();
const GLFWvidmode* mode = glfwGetVideoMode(monitor);
unsigned int SCR_W      = mode->width;
unsigned int SCR_H      = mode->height;

glfwWindowHint(GLFW_RED_BITS,     mode->redBits);
glfwWindowHint(GLFW_GREEN_BITS,   mode->greenBits);
glfwWindowHint(GLFW_BLUE_BITS,    mode->blueBits);
glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

   GLFWwindow* window = glfwCreateWindow(SCR_W, SCR_H, "Microverse — Virtual Lab", monitor, nullptr);
    glfwMakeContextCurrent(window);
    // ── Set window icon ───────────────────────────────────
// ── Set window icon ───────────────────────────────────
{
    GLFWimage icon;
    stbi_set_flip_vertically_on_load(false);
    icon.pixels = stbi_load("assets/logo.png",
                            &icon.width, &icon.height, nullptr, 4);
    if (icon.pixels) {
        glfwSetWindowIcon(window, 1, &icon);
        stbi_image_free(icon.pixels);
    }
    stbi_set_flip_vertically_on_load(true);
}
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    stbi_set_flip_vertically_on_load(true);

    // ── Load Shaders ──────────────────────────────────────
    Shader lightShader   ("shaders/lighting.vert",    "shaders/lighting.frag");
    Shader postShader    ("shaders/postprocess.vert", "shaders/postprocess.frag");
    Shader skyboxShader  ("shaders/skybox.vert",      "shaders/skybox.frag");
    Shader outlineShader ("shaders/lighting.vert",    "shaders/stencil_outline.frag");
    Shader emissiveShader("shaders/emissive.vert",    "shaders/emissive.frag");
    Shader roomShader    ("shaders/room.vert",        "shaders/room.frag");
    Shader objectShader  ("shaders/object.vert",      "shaders/object.frag");
    Shader hologramShader("shaders/hologram.vert",    "shaders/hologram.frag");
    Shader hudShader     ("shaders/hud.vert",         "shaders/hud.frag");
    Shader textShader    ("shaders/text.vert",        "shaders/text.frag");

    // ── Text projection (ortho, set sekali) ──────────────
    glm::mat4 textProj = glm::ortho(0.0f, (float)SCR_W, 0.0f, (float)SCR_H);
    textShader.use();
    textShader.setMat4("projection", textProj);

    // ── Load Models ───────────────────────────────────────
    Model virusModel("models/nukleus/nukleus.obj");
    Model mitoModel ("models/mitokondria/mitokondria.obj");

    // ── Hologram ──────────────────────────────────────────
    HologramText welcomeHologram;
    if (!welcomeHologram.init("textures/hologram/welcome.png"))
        std::cout << "Warning: Hologram image not found!\n";

    // ── Interaction & HUD ─────────────────────────────────
    InteractionSystem interactSys;
    interactSys.init();
    g_interactSys = &interactSys;  // sambungkan ke global pointer

    InfoPanel infoPanel;
    infoPanel.init(SCR_W, SCR_H);
    g_infoPanel = &infoPanel;   // ← TAMBAH

    // ── Text Renderer ─────────────────────────────────────
    TextRenderer textRenderer;
    textRenderer.Init(SCR_W, SCR_H);
    if (!textRenderer.Load("fonts/Grotesk.otf", 24))
        std::cout << "Warning: Font tidak ditemukan!\n";

    // ── Hitung AABB model ─────────────────────────────────
    auto computeModelAABB = [&](Model& m, glm::vec3& outMin, glm::vec3& outMax) {
        bool init = false;
        for (auto& mesh : m.meshes)
            for (auto& v : mesh.vertices) {
                if (!init) { outMin = outMax = v.Position; init = true; }
                outMin = glm::min(outMin, v.Position);
                outMax = glm::max(outMax, v.Position);
            }
        if (!init) { outMin = outMax = glm::vec3(0.0f); }
    };

    glm::vec3 mitoMin,  mitoMax,  mitoCenter;
    glm::vec3 virusMin, virusMax, virusCenter;
    computeModelAABB(mitoModel,  mitoMin,  mitoMax);
    computeModelAABB(virusModel, virusMin, virusMax);
    mitoCenter  = (mitoMin  + mitoMax)  * 0.5f;
    virusCenter = (virusMin + virusMax) * 0.5f;

    float mitoMaxDim  = std::max({(mitoMax  - mitoMin).x,  (mitoMax  - mitoMin).y,  (mitoMax  - mitoMin).z});
    float virusMaxDim = std::max({(virusMax - virusMin).x, (virusMax - virusMin).y, (virusMax - virusMin).z});
    float targetDiameter  = 3.2f;
    float mitoAutoScale   = mitoMaxDim  > 0.0f ? (targetDiameter / mitoMaxDim)  : 1.0f;
    float virusAutoScale  = virusMaxDim > 0.0f ? (2.0f / virusMaxDim) : 1.0f;
    const float platformTopY = 0.35f;
    float mitoYOffset  = platformTopY + (-mitoMin.y)  * mitoAutoScale  + 0.9f;
    float virusYOffset = platformTopY + (-virusMin.y) * virusAutoScale + 0.4f;

    // ── Lab ───────────────────────────────────────────────
    Lab lab;
    lab.init();

    // ── Framebuffer post-processing ───────────────────────
    unsigned int fbo, fboTex, rbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGenTextures(1, &fboTex);
    glBindTexture(GL_TEXTURE_2D, fboTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_W, SCR_H, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTex, 0);
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_W, SCR_H);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "\n=== MICROVERSE Virtual Lab ===\n";
    std::cout << "WASD: gerak | Mouse: lihat | F: toggle Walk/Fly\n";
    std::cout << "Space: lompat | P: post-effect | E: Interact\n";
    std::cout << "Dekat objek -> tekan E -> popup\n";
    std::cout << "< / > navigasi slide | Q / ESC tutup popup\n\n";

    float hologramAppearTime = 0.0f;
    bool  hologramAppeared   = false;

    // ── FPS tracking ──────────────────────────────────────
    float fpsTimer  = 0.0f;
    int   fpsCount  = 0;

    // ════════════════════════════════════════════════════════
    // RENDER LOOP
    // ════════════════════════════════════════════════════════
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = (float)glfwGetTime();
        deltaTime  = currentFrame - lastFrame;
        lastFrame  = currentFrame;

        processInput(window);
        camera.UpdatePhysics(deltaTime);

        // ── Update interaction ─────────────────────────────
        interactSys.update(camera.Position, deltaTime);

        // ── Hologram appear ───────────────────────────────
        if (!hologramAppeared && currentFrame > 0.5f) {
            hologramAppearTime = currentFrame;
            hologramAppeared   = true;
        }

        // ── FPS title ─────────────────────────────────────
        fpsTimer += deltaTime;
        fpsCount++;
        if (fpsTimer >= 1.0f) {
            glfwSetWindowTitle(window,
                ("Microverse — Virtual Lab | FPS: " + std::to_string(fpsCount)).c_str());
            fpsTimer = 0.0f; fpsCount = 0;
        }

        // ════════════════════════════════════════════════════
        // PASS 1: Render scene ke FBO
        // ════════════════════════════════════════════════════
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);
        // Ganti clearColor (sekitar line: glClearColor)
glClearColor(0.005f, 0.005f, 0.012f, 1.0f);  // hampir hitam
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glm::mat4 proj = glm::perspective(glm::radians(camera.Zoom),
                                          (float)SCR_W / (float)SCR_H, 0.1f, 200.0f);
        glm::mat4 view = camera.GetViewMatrix();

        glm::vec3 stagePosArr[3] = {
            {-6.0f, 0.0f, -5.0f},
            { 6.0f, 0.0f, -5.0f},
            { 0.0f, 0.0f,  8.0f},
        };

        // STEP 1: Ruangan
        glDisable(GL_CULL_FACE);
        roomShader.use();
        roomShader.setMat4("projection", proj);
        roomShader.setMat4("view",       view);
        roomShader.setVec3("viewPos",    camera.Position);
        glm::vec3 neonPos[8] = {
            {-13.0f,1.0f,  0.0f},{-13.0f,2.5f, -8.0f},{-13.0f,4.5f,  8.0f},
            { 13.0f,1.0f,  0.0f},{ 13.0f,2.5f,  8.0f},{ 13.0f,4.5f, -8.0f},
            {  0.0f,7.8f,  0.0f},{  0.0f,7.8f,-12.0f},
        };
// Ganti lightRadius dari 6.0f → 3.5f, dan kurangi intensitas
glm::vec3 neonCol(0.55f, 0.75f, 1.0f);  // lebih biru, kurang putih
roomShader.setInt("numLights", 8);
for (int i = 0; i < 8; i++) {
    roomShader.setVec3 ("lightPos["   +std::to_string(i)+"]", neonPos[i]);
    roomShader.setVec3 ("lightColor[" +std::to_string(i)+"]", neonCol);
    roomShader.setFloat("lightRadius["+std::to_string(i)+"]", 3.2f);  // was 6.0f
}
        lab.draw(roomShader);

        // STEP 2: Neon + glow
        emissiveShader.use();
        emissiveShader.setMat4("projection", proj);
        emissiveShader.setMat4("view",       view);
        lab.drawEmissive(emissiveShader);

        // STEP 3: Model 3D
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        objectShader.use();
        objectShader.setMat4("projection", proj);
        objectShader.setMat4("view",       view);
        objectShader.setVec3("viewPos",    camera.Position);

        // Mitokondria
        {
            glm::vec3 pos    = stagePosArr[0];
            glm::vec3 center = glm::vec3(pos.x, pos.y + mitoYOffset, pos.z);
            glm::mat4 model  = glm::mat4(1.0f);
            model = glm::translate(model, center);
            model = glm::rotate(model, currentFrame * 0.3f, glm::vec3(0,1,0));
            model = glm::scale(model, glm::vec3(mitoAutoScale));
            model = glm::translate(model, -mitoCenter);
            objectShader.setMat4("model",        model);
            objectShader.setVec3("objectCenter", center);
            mitoModel.Draw(objectShader);
        }

        // Nukleus
        {
            glm::vec3 pos    = stagePosArr[1];
            glm::vec3 center = glm::vec3(pos.x, pos.y + virusYOffset, pos.z);
            glm::mat4 model  = glm::mat4(1.0f);
            model = glm::translate(model, center);
            model = glm::rotate(model, currentFrame * 0.25f, glm::vec3(0,1,0));
            model = glm::scale(model, glm::vec3(virusAutoScale));
            model = glm::translate(model, -virusCenter);
            objectShader.setMat4("model",        model);
            objectShader.setVec3("objectCenter", center);
            virusModel.Draw(objectShader);
        }

        // STEP 4: Skybox
        glDisable(GL_CULL_FACE);
        lab.drawSkybox(skyboxShader, view, proj);

        // STEP 4.5: Hologram
// Di main.cpp, bagian render hologram:

// STEP 4.5: Hologram
if (hologramAppeared) {
    welcomeHologram.updateCameraPos(camera.Position);
    
    // JANGAN disable depth test!
    // glDisable(GL_DEPTH_TEST);  // ← HAPUS INI!
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // TAPI pake depth test dengan alpha testing biar ga nembus
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);  // depth test normal
    
    float timeSinceAppear = currentFrame - hologramAppearTime;
    float floatY   = sinf(currentFrame * 1.5f) * 0.08f;
    float rotY     = currentFrame * 0.15f;
    float fadeAlpha = 0.85f;
    if (timeSinceAppear < 2.0f)
        fadeAlpha = (timeSinceAppear / 2.0f) * 0.85f;
    float scaleIn = (timeSinceAppear < 1.0f)
                    ? std::max(0.3f, timeSinceAppear) : 1.0f;

    welcomeHologram.render(hologramShader, view, proj,
        glm::vec3(0.0f, 1.3f + floatY, 8.0f),
        2.2f * scaleIn, fadeAlpha, currentFrame, rotY);

    // glEnable(GL_DEPTH_TEST);  // ← SUDAH ENABLE, ga usah di-enable lagi
}
        // ════════════════════════════════════════════════════
        // PASS 2: Post-processing
        // ════════════════════════════════════════════════════
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glClear(GL_COLOR_BUFFER_BIT);

        postShader.use();
        postShader.setInt("screenTexture", 0);
        postShader.setInt("effect",        postEffect);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fboTex);
        renderQuad();

        // ════════════════════════════════════════════════════
        // PASS 3: HUD — panel background (quad)
        // ════════════════════════════════════════════════════
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        infoPanel.render(hudShader, interactSys, currentFrame);

        // ── Teks HUD via TextRenderer ──────────────────────
        textShader.use();
        textShader.setMat4("projection", textProj);

        // Teks dari InfoPanel (prompt + popup content)
        auto textEntries = infoPanel.getTextEntries(interactSys, currentFrame);
        for (auto& t : textEntries) {
            textRenderer.RenderText(textShader,
                                    t.text, t.x, t.y, t.scale,
                                    t.color, t.alpha);
        }

        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}