// rendering/HologramText.h
#pragma once
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <stb_image.h>

class HologramText {
public:
    HologramText() : VAO(0), VBO(0), textureID(0), m_width(1.0f), m_height(1.0f) {}
    
    bool init(const char* imagePath) {
        // Load gambar
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        int width, height, nrChannels;
        unsigned char* data = stbi_load(imagePath, &width, &height, &nrChannels, 4); // Force RGBA
        
        if (data) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
            
            // Simpan aspect ratio gambar asli
            m_aspectRatio = (float)width / (float)height;
            m_width = 1.0f;
            m_height = 1.0f / m_aspectRatio;
            
            std::cout << "Loaded hologram: " << imagePath << " (" << width << "x" << height << ")" << std::endl;
            std::cout << "Aspect ratio: " << m_aspectRatio << std::endl;
        } else {
            std::cout << "Failed to load hologram: " << imagePath << std::endl;
            return false;
        }
        
        setupQuad();
        return true;
    }
    
    void render(Shader& shader, glm::mat4 view, glm::mat4 proj, 
                glm::vec3 position, float scale = 1.0f,
                float alpha = 0.85f, float time = 0.0f,
                float rotation = 0.0f) {
        
        shader.use();
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, position);
        
        // Rotation (hanya di sumbu Y)
        model = glm::rotate(model, rotation, glm::vec3(0.0f, 1.0f, 0.0f));
        
        // ─── BILLBOARD: selalu menghadap kamera ───
        glm::mat4 viewNoTranslate = glm::mat4(glm::mat3(view));
        glm::vec3 right = glm::vec3(viewNoTranslate[0][0], viewNoTranslate[1][0], viewNoTranslate[2][0]);
        glm::vec3 up = glm::vec3(viewNoTranslate[0][1], viewNoTranslate[1][1], viewNoTranslate[2][1]);
        
        // ⚠️ UKURAN KONSISTEN: tidak tergantung jarak!
        // Gunakan fixed size di world space
        float worldWidth = scale * m_width;
        float worldHeight = scale * m_height;
        
        // Build billboard matrix dengan ukuran FIXED
        glm::mat4 billboard = glm::mat4(1.0f);
        billboard[0][0] = right.x * worldWidth;
        billboard[0][1] = right.y * worldWidth;
        billboard[0][2] = right.z * worldWidth;
        billboard[1][0] = up.x * worldHeight;
        billboard[1][1] = up.y * worldHeight;
        billboard[1][2] = up.z * worldHeight;
        
        model = model * billboard;
        
        shader.setMat4("model", model);
        shader.setMat4("view", view);
        shader.setMat4("projection", proj);
        shader.setInt("textTexture", 0);
        shader.setFloat("alpha", alpha);
        shader.setFloat("time", time);
        shader.setFloat("glowIntensity", 0.6f);
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    
    void updateCameraPos(glm::vec3 pos) { cameraPos = pos; }
    
    // Getter untuk aspect ratio
    float getAspectRatio() const { return m_aspectRatio; }
    float getWidth() const { return m_width; }
    float getHeight() const { return m_height; }
    
private:
    unsigned int VAO, VBO, textureID;
    glm::vec3 cameraPos;
    float m_aspectRatio;
    float m_width;
    float m_height;
    
    void setupQuad() {
        // Quad dengan ukuran 1x1 (akan di-scale sesuai aspect ratio di render)
        float vertices[] = {
            -0.5f,  0.5f,   0.0f, 1.0f,
            -0.5f, -0.5f,   0.0f, 0.0f,
             0.5f, -0.5f,   1.0f, 0.0f,
             
            -0.5f,  0.5f,   0.0f, 1.0f,
             0.5f, -0.5f,   1.0f, 0.0f,
             0.5f,  0.5f,   1.0f, 1.0f
        };
        
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        
        glBindVertexArray(0);
    }
};