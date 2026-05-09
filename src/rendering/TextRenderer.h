#pragma once
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <map>
#include <string>
#include <iostream>
#include "../core/Shader.h"

struct Character {
    unsigned int TextureID;
    glm::ivec2   Size;
    glm::ivec2   Bearing;
    unsigned int Advance;
};

class TextRenderer {
public:
    std::map<char, Character> Characters;
    unsigned int VAO, VBO;
    unsigned int SCR_W, SCR_H;

    void Init(unsigned int screenW, unsigned int screenH) {
        SCR_W = screenW;
        SCR_H = screenH;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    bool Load(const std::string& fontPath, unsigned int fontSize) {
        Characters.clear();

        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            std::cerr << "[TextRenderer] FreeType init gagal!\n";
            return false;
        }

        FT_Face face;
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
            std::cerr << "[TextRenderer] Font tidak ditemukan: " << fontPath << "\n";
            FT_Done_FreeType(ft);
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, fontSize);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cerr << "[TextRenderer] Gagal load karakter: " << c << "\n";
                continue;
            }

            unsigned int tex;
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                         face->glyph->bitmap.width,
                         face->glyph->bitmap.rows,
                         0, GL_RED, GL_UNSIGNED_BYTE,
                         face->glyph->bitmap.buffer);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Characters[c] = {
                tex,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left,  face->glyph->bitmap_top),
                (unsigned int)face->glyph->advance.x
            };
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        return true;
    }

    // Render teks — koordinat Y dari BAWAH layar (OpenGL style)
void RenderText(Shader& shader, const std::string& text,
                float x, float y, float scale,
                glm::vec3 color, float alpha = 1.0f)
{
    if (Characters.empty()) {
        std::cout << "[TextRenderer] ERROR: Characters kosong!\n";
        return;
    }

    shader.use();
    shader.setVec3("textColor", color);
    shader.setFloat("textAlpha", alpha);
    shader.setInt("text", 0);          // ← FIX 1: set texture unit

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (char c : text) {
        if (Characters.find(c) == Characters.end()) continue;
        Character& ch = Characters[c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;
        float w    = ch.Size.x * scale;
        float h    = ch.Size.y * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos,     ypos,     0.0f, 1.0f },
            { xpos + w, ypos,     1.0f, 1.0f },
            { xpos,     ypos + h, 0.0f, 0.0f },
            { xpos + w, ypos,     1.0f, 1.0f },
            { xpos + w, ypos + h, 1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // FIX 2: bind VBO, upload, JANGAN unbind sebelum draw
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        // ← TIDAK ada glBindBuffer(..., 0) di sini
        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  // unbind SETELAH semua draw selesai
    glBindTexture(GL_TEXTURE_2D, 0);
}
};