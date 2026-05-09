#pragma once
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include "../core/Shader.h"
#include "InteractionSystem.h"

// ============================================================================
// TEXT ENTRY STRUCT - Dipake buat render teks di main.cpp
// ============================================================================
struct TextEntry {
    std::string text;
    float x, y, scale;
    glm::vec3 color;
    float alpha;
};

// ============================================================================
// DESIGN SYSTEM - Professional UI Tokens
// ============================================================================
struct DesignTokens {
    static constexpr float BASE_W = 1920.0f;
    static constexpr float BASE_H = 1080.0f;
    static constexpr float GRID = 8.0f;
    
    static constexpr float SPACE_XS = 0.5f;
    static constexpr float SPACE_SM = 1.0f;
    static constexpr float SPACE_MD = 2.0f;
    static constexpr float SPACE_LG = 3.0f;
    static constexpr float SPACE_XL = 4.0f;
    
    static constexpr float FONT_TINY = 0.28f;
    static constexpr float FONT_SM = 0.36f;
    static constexpr float FONT_BASE = 0.48f;
    static constexpr float FONT_MD = 0.62f;
    static constexpr float FONT_LG = 0.82f;
    static constexpr float FONT_XL = 1.05f;
    static constexpr float FONT_XXL = 1.35f;
    
    struct Colors {
        static glm::vec4 primary()    { return {0.08f, 0.65f, 0.95f, 1.0f}; }
        static glm::vec4 primaryDark(){ return {0.05f, 0.35f, 0.75f, 1.0f}; }
        static glm::vec4 surface()    { return {0.02f, 0.04f, 0.10f, 0.95f}; }
        static glm::vec4 surfaceLight(){return {0.04f, 0.08f, 0.16f, 0.95f};}
        static glm::vec4 text()       { return {0.92f, 0.95f, 1.00f, 1.0f}; }
        static glm::vec4 textDim()    { return {0.55f, 0.65f, 0.85f, 1.0f}; }
        static glm::vec4 border()     { return {0.12f, 0.45f, 0.85f, 0.5f}; }
    };
};

// ============================================================================
// BUTTON DEFINITION
// ============================================================================
struct UIButton {
    enum Type { PRIMARY, NAV_LEFT, NAV_RIGHT, CLOSE };
    
    Type type;
    glm::vec2 pos;
    glm::vec2 size;
    float radius;
    std::string icon;
    bool enabled;
    
    UIButton(Type t, float x, float y, float w, float h, float r, const std::string& ic = "")
        : type(t), pos(x, y), size(w, h), radius(r), icon(ic), enabled(true) {}
    
    bool contains(float px, float py) const {
        return enabled && px >= pos.x && px <= pos.x + size.x &&
               py >= pos.y && py <= pos.y + size.y;
    }
    
    glm::vec4 getBgColor(float alpha = 1.0f) const {
        if (!enabled) return {0.2f, 0.2f, 0.3f, alpha * 0.3f};
        if (type == CLOSE) return {0.55f, 0.10f, 0.10f, alpha};
        return {0.04f, 0.07f, 0.14f, alpha};
    }
    
    glm::vec4 getBorderColor(float alpha = 1.0f) const {
        if (!enabled) return {0.3f, 0.3f, 0.4f, alpha * 0.3f};
        if (type == CLOSE) return {0.85f, 0.25f, 0.25f, alpha};
        return DesignTokens::Colors::border();
    }
};

// ============================================================================
// LAYOUT MANAGER
// ============================================================================
class Layout {
public:
    Layout(float screenW, float screenH) : w(screenW), h(screenH) {
        scale = std::min(screenW / DesignTokens::BASE_W, 
                         screenH / DesignTokens::BASE_H);
        unit = DesignTokens::GRID * scale;
    }
    
    float u(float n = 1.0f) const { return n * unit; }
    float fs(float baseScale) const { return std::clamp(baseScale * scale, 0.45f, 2.0f); }
    
    struct Rect { float x, y, w, h; };
    
    Rect dialog(float wu = 115.0f, float hu = 72.0f) const {
        float pw = u(wu), ph = u(hu);
        return { (w - pw) / 2.0f, (h - ph) / 2.0f, pw, ph };
    }
    
private:
    float w, h, scale, unit;
};

// ============================================================================
// INFO PANEL - MAIN CLASS
// ============================================================================
class InfoPanel {
public:
    void init(unsigned int scrW, unsigned int scrH) {
        m_width = scrW;
        m_height = scrH;
        m_layout = Layout((float)scrW, (float)scrH);
        setupQuad();
    }
    
    void render(Shader& shader, InteractionSystem& is, float time) {
        m_time = time;
        m_pulse = 0.85f + std::sin(time * 5.0f) * 0.1f;
        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glm::mat4 ortho = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height);
        shader.use();
        shader.setMat4("projection", ortho);
        
        if (is.promptVisible && is.getNearestObject()) {
            renderPrompt(shader, is);
        }
        
        if ((is.popupOpen || is.popupClosing) && is.popupAlpha > 0.01f && is.getNearestObject()) {
            renderPopup(shader, is);
        }
        
        glEnable(GL_DEPTH_TEST);
    }
    
    // INI YANG DIPAKE MAIN.CPP - NGEMBALIKIN VECTOR<TextEntry>
    std::vector<TextEntry> getTextEntries(InteractionSystem& is, float time) {
        std::vector<TextEntry> entries;
        
        if (is.promptVisible && is.getNearestObject()) {
            addPromptText(entries, is);
        }
        
        if ((is.popupOpen || is.popupClosing) && is.popupAlpha > 0.01f && is.getNearestObject()) {
            addPopupText(entries, is);
        }
        
        return entries;
    }
    
    bool handleMouseClick(InteractionSystem& is, float mouseX, float mouseY) {
        if (!is.popupOpen || !is.getNearestObject()) return false;
        
        auto* obj = is.getNearestObject();
        int total = (int)obj->slides.size();
        float glY = m_height - mouseY;
        auto panel = m_layout.dialog();
        
        // Navigation buttons
        float navW = m_layout.u(6.0f);
        float navH = m_layout.u(9.0f);
        float navY = panel.y + panel.h / 2.0f - navH / 2.0f;
        
        UIButton prev(UIButton::NAV_LEFT, panel.x + m_layout.u(1.25f), navY, navW, navH, m_layout.u(1.0f), "‹");
        UIButton next(UIButton::NAV_RIGHT, panel.x + panel.w - m_layout.u(7.5f), navY, navW, navH, m_layout.u(1.0f), "›");
        UIButton close(UIButton::CLOSE, panel.x + panel.w - m_layout.u(6.5f), panel.y + panel.h - m_layout.u(7.75f), 
                       m_layout.u(5.0f), m_layout.u(5.0f), m_layout.u(0.875f), "✕");
        
        prev.enabled = (is.currentSlide > 0);
        next.enabled = (is.currentSlide < total - 1);
        
        if (prev.enabled && prev.contains(mouseX, glY)) {
            if (is.currentSlide > 0) is.currentSlide--;
            return true;
        }
        if (next.enabled && next.contains(mouseX, glY)) {
            if (is.currentSlide < total - 1) is.currentSlide++;
            return true;
        }
        if (close.contains(mouseX, glY)) {
            is.closePopup();
            return true;
        }
        
        return false;
    }

private:
    unsigned int m_width = 1920, m_height = 1080;
    unsigned int m_quadVAO = 0, m_quadVBO = 0;
    Layout m_layout = Layout(1920, 1080);
    float m_time = 0.0f;
    float m_pulse = 1.0f;
    
// ========================================================================
// PROMPT RENDERING - VERSION FIX (lebih pendek & rapi)
// ========================================================================
void renderPrompt(Shader& shader, InteractionSystem& is) {
    float a = is.promptAlpha;
    float sc = is.promptScale;
    
    float cx = m_width / 2.0f;
    float cy = m_layout.u(18.0f);  // Naikkin dikit
    
    // PILL LEBIH PENDEK: 320px (dari 400)
    float pw = 200.0f * sc;
    float ph = 56.0f * sc;  // Lebih pendek juga
    float px = cx - pw / 2.0f;
    float py = cy - ph / 2.0f;
    
    // Background
    drawRect(shader, px - 10, py - 6, pw + 20, ph + 12,
             {0.05f, 0.35f, 0.90f, 0.12f * m_pulse * a}, 16.0f);
    drawRect(shader, px, py, pw, ph, {0.02f, 0.04f, 0.10f, 0.94f * a}, 10.0f);
    drawRectBorder(shader, px, py, pw, ph, {0.12f, 0.65f, 1.00f, 0.55f * m_pulse * a}, 1.5f, 10.0f);
    
    // Kotak E - lebih kecil dikit
    float boxS = 44.0f * sc;
    float boxX = px + 10.0f;
    float boxY = py + (ph - boxS) / 2.0f;
    drawRect(shader, boxX, boxY, boxS, boxS, {0.06f, 0.20f, 0.55f, 0.96f * a}, 8.0f);
    drawRectBorder(shader, boxX, boxY, boxS, boxS, {0.20f, 0.85f, 1.00f, 0.90f * m_pulse * a}, 1.5f, 8.0f);
    
    // Separator
    drawRect(shader, boxX + boxS + 8.0f, py + 6.0f, 1.5f, ph - 12.0f,
             {0.15f, 0.60f, 0.95f, 0.40f * a}, 0);
}

void addPromptText(std::vector<TextEntry>& entries, InteractionSystem& is) {
    auto* obj = is.getNearestObject();
    float a = is.promptAlpha;
    float sc = is.promptScale;
    
    float cx = m_width / 2.0f;
    float cy = m_layout.u(18.0f);
    
    float pw = 200.0f * sc;
    float ph = 56.0f * sc;
    float px = cx - pw / 2.0f;
    float py = cy - ph / 2.0f;
    
    float boxS = 44.0f * sc;
    float boxX = px + 10.0f;
    float boxY = py + (ph - boxS) / 2.0f;
    
    // Tombol E
    entries.push_back({"E", 
        boxX + boxS/2.0f - 10.0f, 
        boxY + boxS/2.0f + -10.0f,
        m_layout.fs(1.75f) * sc,
        {0.28f, 0.90f, 1.00f}, a * m_pulse});
    
    float textX = boxX + boxS + 8.0f + 12.0f;
    float textBaseY = py + ph - 16.0f;
    
    // GABUNG JADI SATU STRING biar nggak ketumpuk
    std::string fullLabel = "Pelajari  " + obj->name;
    
    entries.push_back({fullLabel,
        textX, textBaseY - 2.0f,
        m_layout.fs(0.58f) * sc,
        {0.92f, 0.96f, 1.00f}, a});
    
    // Hint
    entries.push_back({"Tekan [E]",
        textX, textBaseY - 18.0f,
        m_layout.fs(0.30f) * sc,
        {0.55f, 0.70f, 0.92f}, a * 0.85f});
}
    // ========================================================================
    // POPUP RENDERING
    // ========================================================================
    void renderPopup(Shader& shader, InteractionSystem& is) {
        float a = is.popupAlpha;
        auto panel = m_layout.dialog();
        auto* obj = is.getNearestObject();
        int total = (int)obj->slides.size();
        
        // Backdrop
        drawRect(shader, 0, 0, (float)m_width, (float)m_height,
                 {0.00f, 0.01f, 0.06f, 0.85f * a});
        
        // Scanlines
        for (int i = 0; i < 18; i++) {
            drawRect(shader, 0, panel.y + i * (panel.h / 18.0f), 
                     (float)m_width, 1.0f, {0.10f, 0.45f, 0.90f, 0.03f * a});
        }
        
        // Outer glow
        float glow = m_layout.u(2.0f);
        drawRect(shader, panel.x - glow, panel.y - glow, panel.w + glow * 2, panel.h + glow * 2,
                 {0.04f, 0.30f, 0.80f, 0.12f * m_pulse * a}, m_layout.u(2.75f));
        
        // Main panel
        drawRect(shader, panel.x, panel.y, panel.w, panel.h,
                 DesignTokens::Colors::surface(), m_layout.u(1.75f));
        drawRectBorder(shader, panel.x, panel.y, panel.w, panel.h,
                       DesignTokens::Colors::border(), m_layout.u(0.1875f), m_layout.u(1.75f));
        
        // Header
        float headerH = m_layout.u(9.5f);
        drawRect(shader, panel.x, panel.y + panel.h - headerH, panel.w, headerH,
                 DesignTokens::Colors::surfaceLight(), m_layout.u(1.75f));
        
        // Progress bar
        float progW = panel.w - m_layout.u(8.0f);
        float progFill = progW * ((float)(is.currentSlide + 1) / total);
        
        drawRect(shader, panel.x + m_layout.u(4.0f), panel.y + m_layout.u(6.5f),
                 progW, m_layout.u(0.625f), {0.05f, 0.09f, 0.18f, 0.90f * a}, m_layout.u(0.3125f));
        drawRect(shader, panel.x + m_layout.u(4.0f), panel.y + m_layout.u(6.5f),
                 progFill, m_layout.u(0.625f), DesignTokens::Colors::primary(), m_layout.u(0.3125f));
        
// Slide dots
float dotY = panel.y + m_layout.u(1.25f);
float dotSpc = m_layout.u(2.5f);
float dotX0 = panel.x + panel.w / 2.0f - (total * dotSpc) / 2.5f;

for (int i = 0; i < total; i++) {
    bool active = (i == is.currentSlide);
    
    // SEMUA DOT punya lebar SAMA (GAK ADA YANG KEPANJANGAN)
    float dw = m_layout.u(1.5f);  // ← semua dot ukuran sama
    float dx = dotX0 + i * dotSpc;
    
    if (active) {
        // Glow effect untuk active dot (opsional)
        drawRect(shader, dx - m_layout.u(0.25f), dotY - m_layout.u(0.25f),
                 dw + m_layout.u(0.5f), m_layout.u(0.875f) + m_layout.u(0.5f),
                 {0.08f, 0.45f, 0.95f, 0.30f * a}, m_layout.u(0.625f));
    }
    
    drawRect(shader, dx, dotY, dw, m_layout.u(0.875f),
             {active ? 0.20f : 0.08f, 
              active ? 0.72f : 0.25f, 
              active ? 1.00f : 0.45f,
              (active ? 1.0f : 0.40f) * a}, m_layout.u(0.4375f));
}
        
        // Fun fact bar
        float factH = m_layout.u(6.5f);
        drawRect(shader, panel.x + m_layout.u(4.0f),
         panel.y + m_layout.u(9.0f),
                 panel.w - m_layout.u(8.0f),
                  factH, {0.03f, 0.09f, 0.07f, 0.80f * a}, m_layout.u(0.875f));
    }
    

void addPopupText(std::vector<TextEntry>& entries, InteractionSystem& is) {
    auto* obj = is.getNearestObject();
    float a   = is.popupAlpha;
    auto panel = m_layout.dialog();
    auto& slide = obj->slides[is.currentSlide];
    int total   = (int)obj->slides.size();

    // ── Koordinat tetap berbasis panel ───────────────────
    float leftX  = panel.x + 22.0f;
    float rightX = panel.x + panel.w - 22.0f;
    float topY   = panel.y + panel.h;   // Y tertinggi panel
    float botY   = panel.y;             // Y terbawah panel

    // ════════ HEADER (zona: topY-88 .. topY) ════════════
        entries.push_back({obj->name, panel.x + m_layout.u(3.0f), 
            panel.y + panel.h - m_layout.u(4.5f), m_layout.fs(DesignTokens::FONT_XL),
            {0.22f, 0.80f, 1.00f}, a});
        
        entries.push_back({obj->subtitle, panel.x + m_layout.u(3.0f), 
            panel.y + panel.h - m_layout.u(7.75f), m_layout.fs(DesignTokens::FONT_SM),
            {0.45f, 0.68f, 0.95f}, a * 0.85f});
        
        entries.push_back({std::to_string(is.currentSlide + 1) + " / " + std::to_string(total),
            panel.x + panel.w - m_layout.u(6.5f), panel.y + panel.h - m_layout.u(5.5f),
            m_layout.fs(DesignTokens::FONT_BASE), {0.35f, 0.62f, 0.95f}, a});

    // ════════ JUDUL SLIDE (zona: topY-108 .. topY-88) ════
    entries.push_back({slide.title,
        panel.x + panel.w / 2.0f - (float)slide.title.size() * 5.0f,
        topY - 95.0f,
        m_layout.fs(DesignTokens::FONT_XL),
        {1.00f, 0.96f, 0.82f}, a});

    // Garis dekoratif bawah judul
    entries.push_back({"- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -",
        leftX + 130.0f,
        topY - 110.0f,
        m_layout.fs(0.22f),
        {0.15f, 0.50f, 0.80f}, a * 0.45f});

    // ════════ BODY TEXT (zona: topY-148 ke bawah, max 4 baris) ══
    // Limit wrap lebih ketat + max 4 baris
// Kasih spasi manual di wrapText-nya

auto lines = wrapText(slide.body, 60);  // Lebih pendek biar ga ada sisa
if (lines.size() > 3) lines.resize(5);   // Cuma 3 baris biar keliatan rapi

float lineStartY = topY - 152.0f;
float lineH = 20.0f;

// Margin yang sama kiri-kanan
float margin = 152.5f;  // ← GANTI ANGKA INI (20, 30, 40, 50, 60)
float textLeftX = panel.x + margin;
float textRightX = panel.x + panel.w - margin;

for (size_t i = 0; i < lines.size(); i++) {
    entries.push_back({lines[i],
        textLeftX,
        lineStartY - (float)i * lineH,
        m_layout.fs(0.34f),
        {0.84f, 0.91f, 1.00f}, a * 0.95f});
}

    // ════════ FUN FACT (zona tetap: botY+70 .. botY+130) ═════
    // Header fakta
    entries.push_back({"FAKTA MENARIK",
        leftX + 0.0f,
        botY + 90.0f,
        m_layout.fs(0.28f),
        {0.22f, 0.92f, 0.50f}, a});

    // Isi fakta — 1 baris saja (potong kalau kepanjangan)
    std::string factShort = slide.fact;
    if (factShort.size() > 72) factShort = factShort.substr(0, 69) + "...";
    entries.push_back({factShort,
        leftX + 10.0f,
        botY + 60.0f,
        m_layout.fs(0.30f),
        {1.00f, 0.88f, 0.38f}, a * 0.92f});

    // ════════ NAVIGASI (zona: botY+16 .. botY+36) ════════
    if (is.currentSlide > 0)
        entries.push_back({"<",
            panel.x + 30.0f,
            panel.y + panel.h / 2.0f - 10.0f,
            m_layout.fs(0.65f),
            {0.30f, 0.82f, 1.00f}, a});

    if (is.currentSlide < total - 1)
        entries.push_back({">",
            panel.x + panel.w - 52.0f,
            panel.y + panel.h / 2.0f - 10.0f,
            m_layout.fs(0.65f),
            {0.30f, 0.82f, 1.00f}, a});



    // Hint kontrol bawah
    entries.push_back({"< >  Navigasi     Q  Tutup",
        panel.x + panel.w / 2.0f - 60.0f,
        botY + 20.0f,
        m_layout.fs(0.26f),
        {0.38f, 0.55f, 0.82f}, a * 0.70f});
}
    // ========================================================================
    // RENDER UTILITIES
    // ========================================================================
    void setupQuad() {
        float v[] = {0,0,0,0, 1,0,1,0, 1,1,1,1, 1,1,1,1, 0,1,0,1, 0,0,0,0};
        glGenVertexArrays(1, &m_quadVAO);
        glGenBuffers(1, &m_quadVBO);
        glBindVertexArray(m_quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindVertexArray(0);
    }
    
    void drawRect(Shader& s, float x, float y, float w, float h, glm::vec4 color, float radius = 0.0f) {
        glm::mat4 m = glm::translate(glm::mat4(1.0f), {x, y, 0});
        m = glm::scale(m, {w, h, 1});
        s.setMat4("model", m);
        s.setVec4("color", color);
        s.setFloat("radius", radius);
        s.setVec2("rectSize", {w, h});
        glBindVertexArray(m_quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    void drawRectBorder(Shader& s, float x, float y, float w, float h, glm::vec4 color, float t, float r = 0.0f) {
        drawRect(s, x, y + h - t, w, t, color, r);
        drawRect(s, x, y, w, t, color, r);
        drawRect(s, x, y, t, h, color, r);
        drawRect(s, x + w - t, y, t, h, color, r);
    }
    
    std::vector<std::string> wrapText(const std::string& text, int maxChars) {
        std::vector<std::string> lines;
        std::string cur;
        std::istringstream iss(text);
        std::string word;
        while (iss >> word) {
            if ((int)(cur.size() + word.size() + 1) > maxChars && !cur.empty()) {
                lines.push_back(cur);
                cur = word;
            } else {
                if (!cur.empty()) cur += " ";
                cur += word;
            }
        }
        if (!cur.empty()) lines.push_back(cur);
        return lines;
    }
};