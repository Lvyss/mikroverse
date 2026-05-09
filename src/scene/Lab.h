#pragma once
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include "../core/Shader.h"

class Lab {
public:
    struct SpotLight {
        glm::vec3 position, direction, color;
        float cutOff, outerCutOff, intensity;
    };
    std::vector<SpotLight> spotLights;

    glm::vec3 pointLightPositions[3] = {
        {-6.0f, 5.0f, -5.0f},
        { 6.0f, 5.0f, -5.0f},
        { 0.0f, 5.0f,  8.0f},
    };
    glm::vec3 pointLightColors[3] = {
        {0.3f, 0.7f, 1.0f},
        {0.3f, 0.7f, 1.0f},
        {1.0f, 0.9f, 0.6f},
    };

    void init() {
        setupRoom();
        setupPlatforms();
        setupNeonLines();
        setupGlowRings();
        setupSkybox();

        spotLights = {
            {{-6.0f,5.0f,-5.0f},{0,-1,0},{0.5f,0.8f,1.0f},
             glm::cos(glm::radians(18.0f)),glm::cos(glm::radians(28.0f)),12.0f},
            {{ 6.0f,5.0f,-5.0f},{0,-1,0},{0.5f,0.8f,1.0f},
             glm::cos(glm::radians(18.0f)),glm::cos(glm::radians(28.0f)),12.0f},
            {{ 0.0f,5.0f, 8.0f},{0,-1,0},{1.0f,0.9f,0.6f},
             glm::cos(glm::radians(18.0f)),glm::cos(glm::radians(28.0f)),12.0f},
        };
    }

    void sendLightsToShader(Shader& shader) {
        glm::vec3 neonLightPos[] = {
            {-13.0f,3.0f,  0.0f},
            { 13.0f,3.0f,  0.0f},
            {  0.0f,7.5f,  0.0f},
            {  0.0f,7.5f,-10.0f},
        };
        glm::vec3 neonLightColor = {0.75f,0.9f,1.0f};
        shader.setInt("lightCount", 3);
        for (int i = 0; i < 3; i++) {
            shader.setVec3 ("lightPositions["  +std::to_string(i)+"]", neonLightPos[i]);
            shader.setVec3 ("lightColors["     +std::to_string(i)+"]", neonLightColor);
            shader.setFloat("lightIntensities["+std::to_string(i)+"]", 8.0f);
        }
        shader.setInt("spotLightCount", (int)spotLights.size());
        for (int i = 0; i < (int)spotLights.size(); i++) {
            std::string p = "spotLights["+std::to_string(i)+"].";
            shader.setVec3 (p+"position",    spotLights[i].position);
            shader.setVec3 (p+"direction",   spotLights[i].direction);
            shader.setVec3 (p+"color",       spotLights[i].color);
            shader.setFloat(p+"cutOff",      spotLights[i].cutOff);
            shader.setFloat(p+"outerCutOff", spotLights[i].outerCutOff);
            shader.setFloat(p+"intensity",   spotLights[i].intensity);
        }
    }

    void draw(Shader& shader) {
        drawRoom(shader);
        drawPlatforms(shader);
    }

    void drawEmissive(Shader& emissiveShader) {
        drawNeonLines(emissiveShader);
        drawGlowRings(emissiveShader);
        drawPlatformGlow(emissiveShader); // glow panel emas di platform
    }

    void drawSkybox(Shader& skyboxShader, glm::mat4 view, glm::mat4 proj) {
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        skyboxShader.setMat4("view", glm::mat4(glm::mat3(view)));
        skyboxShader.setMat4("projection", proj);
        skyboxShader.setInt("skybox", 0);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS);
    }

private:
    const float PI = 3.14159265f;

    // VAOs
    unsigned int roomVAO,       roomVBO;
    unsigned int platBodyVAO,   platBodyVBO;   // badan ring utama
    unsigned int platInnerVAO,  platInnerVBO;  // disk tengah
    unsigned int platPanelVAO,  platPanelVBO;  // panel segmen dekoratif
    unsigned int platGlowVAO,   platGlowVBO;   // gold strip emissive
    unsigned int platStepVAO,   platStepVBO;   // step/tangga kecil di belakang
    unsigned int neonVAO,       neonVBO;
    unsigned int ringVAO,       ringVBO;
    unsigned int skyboxVAO,     skyboxVBO;
    unsigned int cubemapTexture;

    unsigned int floorTex, wallTex, ceilTex;
    unsigned int platDarkTex;   // warna gelap metalik platform
    unsigned int platPanelTex;  // warna panel sedikit beda
    unsigned int platInnerTex;  // warna tengah

    int platBodyCount  = 0;
    int platInnerCount = 0;
    int platPanelCount = 0;
    int platGlowCount  = 0;
    int platStepCount  = 0;
    int neonVertCount  = 0;
    int ringVertCount  = 0;
    int neonQuadCount = 0;

    glm::vec3 stagePos[3] = {
        {-6.0f, 0.0f, -5.0f},
        { 6.0f, 0.0f, -5.0f},
        { 0.0f, 0.0f,  8.0f},
    };

    // ── Utility: buat VAO dari vector<float> ─────────────
    void makeVAO(unsigned int& vao, unsigned int& vbo,
                 std::vector<float>& v, int& count,
                 bool hasNormalUV = true)
    {
        count = hasNormalUV ? (int)(v.size()/8) : (int)(v.size()/3);
        glGenVertexArrays(1,&vao); glGenBuffers(1,&vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(float),v.data(),GL_STATIC_DRAW);
        if (hasNormalUV) {
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
        } else {
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
        }
        glBindVertexArray(0);
    }

    // ── Room ─────────────────────────────────────────────
    void setupRoom() {
        float W=28.f,H=8.f,D=36.f,hw=W/2,hd=D/2;
        float v[] = {
            -hw,0,-hd,0,1,0,0,0,  hw,0,-hd,0,1,0,W,0,  hw,0,hd,0,1,0,W,D,
             hw,0,hd,0,1,0,W,D,  -hw,0,hd,0,1,0,0,D,  -hw,0,-hd,0,1,0,0,0,
            -hw,H,-hd,0,-1,0,0,0, hw,H,-hd,0,-1,0,W,0, hw,H,hd,0,-1,0,W,D,
             hw,H,hd,0,-1,0,W,D,-hw,H,hd,0,-1,0,0,D,  -hw,H,-hd,0,-1,0,0,0,
            -hw,0,-hd,1,0,0,0,0, -hw,0,hd,1,0,0,D,0,  -hw,H,hd,1,0,0,D,H,
            -hw,H,hd,1,0,0,D,H,  -hw,H,-hd,1,0,0,0,H, -hw,0,-hd,1,0,0,0,0,
             hw,0,-hd,-1,0,0,0,0, hw,0,hd,-1,0,0,D,0,  hw,H,hd,-1,0,0,D,H,
             hw,H,hd,-1,0,0,D,H,  hw,H,-hd,-1,0,0,0,H, hw,0,-hd,-1,0,0,0,0,
            -hw,0,-hd,0,0,1,0,0,  hw,0,-hd,0,0,1,W,0,  hw,H,-hd,0,0,1,W,H,
             hw,H,-hd,0,0,1,W,H, -hw,H,-hd,0,0,1,0,H, -hw,0,-hd,0,0,1,0,0,
            -hw,0,hd,0,0,-1,0,0,  hw,0,hd,0,0,-1,W,0,  hw,H,hd,0,0,-1,W,H,
             hw,H,hd,0,0,-1,W,H, -hw,H,hd,0,0,-1,0,H, -hw,0,hd,0,0,-1,0,0,
        };
        glGenVertexArrays(1,&roomVAO); glGenBuffers(1,&roomVBO);
        glBindVertexArray(roomVAO);
        glBindBuffer(GL_ARRAY_BUFFER,roomVBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
        glEnableVertexAttribArray(0); glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(3*sizeof(float)));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,8*sizeof(float),(void*)(6*sizeof(float)));
        glBindVertexArray(0);

        floorTex    = loadSolidColor(25, 28, 38);
        wallTex     = loadSolidColor(18, 20, 30);
        ceilTex     = loadSolidColor(12, 14, 22);
        platDarkTex = loadSolidColor(22, 26, 32);  // metalik gelap
        platPanelTex= loadSolidColor(30, 35, 45);  // panel sedikit terang
        platInnerTex= loadSolidColor(15, 18, 25);  // tengah gelap
    }

    void drawRoom(Shader& shader) {
        glm::mat4 m = glm::mat4(1.0f);
        shader.setMat4("model", m);
        bindSolidTex(shader, floorTex);
        glBindVertexArray(roomVAO);
        glDrawArrays(GL_TRIANGLES, 0,  6);
        bindSolidTex(shader, ceilTex);
        glDrawArrays(GL_TRIANGLES, 6,  6);
        bindSolidTex(shader, wallTex);
        glDrawArrays(GL_TRIANGLES, 12, 24);
        glBindVertexArray(0);
    }

    // ── Platform Sci-fi ──────────────────────────────────
    void setupPlatforms() {
        const int SEG = 64;

        // ── 1. Badan ring utama (outer ring tebal) ────────
        {
            std::vector<float> v;
            float rIn=1.5f, rOut=2.1f, h=0.38f;
            for (int i=0;i<SEG;i++) {
                float a0=(float)i/SEG*2*PI, a1=(float)(i+1)/SEG*2*PI;
                float u0=(float)i/SEG,      u1=(float)(i+1)/SEG;
                // Top
                v.insert(v.end(),{rIn*cosf(a0),h,rIn*sinf(a0),  0,1,0, u0,0});
                v.insert(v.end(),{rOut*cosf(a0),h,rOut*sinf(a0), 0,1,0, u0,1});
                v.insert(v.end(),{rOut*cosf(a1),h,rOut*sinf(a1), 0,1,0, u1,1});
                v.insert(v.end(),{rOut*cosf(a1),h,rOut*sinf(a1), 0,1,0, u1,1});
                v.insert(v.end(),{rIn*cosf(a1),h,rIn*sinf(a1),  0,1,0, u1,0});
                v.insert(v.end(),{rIn*cosf(a0),h,rIn*sinf(a0),  0,1,0, u0,0});
                // Outer wall
                v.insert(v.end(),{rOut*cosf(a0),0,rOut*sinf(a0), cosf(a0),0,sinf(a0), u0,0});
                v.insert(v.end(),{rOut*cosf(a0),h,rOut*sinf(a0), cosf(a0),0,sinf(a0), u0,1});
                v.insert(v.end(),{rOut*cosf(a1),h,rOut*sinf(a1), cosf(a1),0,sinf(a1), u1,1});
                v.insert(v.end(),{rOut*cosf(a1),h,rOut*sinf(a1), cosf(a1),0,sinf(a1), u1,1});
                v.insert(v.end(),{rOut*cosf(a1),0,rOut*sinf(a1), cosf(a1),0,sinf(a1), u1,0});
                v.insert(v.end(),{rOut*cosf(a0),0,rOut*sinf(a0), cosf(a0),0,sinf(a0), u0,0});
                // Inner wall
                v.insert(v.end(),{rIn*cosf(a0),0,rIn*sinf(a0), -cosf(a0),0,-sinf(a0), u0,0});
                v.insert(v.end(),{rIn*cosf(a0),h,rIn*sinf(a0), -cosf(a0),0,-sinf(a0), u0,1});
                v.insert(v.end(),{rIn*cosf(a1),h,rIn*sinf(a1), -cosf(a1),0,-sinf(a1), u1,1});
                v.insert(v.end(),{rIn*cosf(a1),h,rIn*sinf(a1), -cosf(a1),0,-sinf(a1), u1,1});
                v.insert(v.end(),{rIn*cosf(a1),0,rIn*sinf(a1), -cosf(a1),0,-sinf(a1), u1,0});
                v.insert(v.end(),{rIn*cosf(a0),0,rIn*sinf(a0), -cosf(a0),0,-sinf(a0), u0,0});
                // Bottom
                v.insert(v.end(),{rIn*cosf(a0),0,rIn*sinf(a0),  0,-1,0, u0,0});
                v.insert(v.end(),{rOut*cosf(a0),0,rOut*sinf(a0), 0,-1,0, u0,1});
                v.insert(v.end(),{rOut*cosf(a1),0,rOut*sinf(a1), 0,-1,0, u1,1});
                v.insert(v.end(),{rOut*cosf(a1),0,rOut*sinf(a1), 0,-1,0, u1,1});
                v.insert(v.end(),{rIn*cosf(a1),0,rIn*sinf(a1),  0,-1,0, u1,0});
                v.insert(v.end(),{rIn*cosf(a0),0,rIn*sinf(a0),  0,-1,0, u0,0});
            }
            makeVAO(platBodyVAO, platBodyVBO, v, platBodyCount);
        }

        // ── 2. Inner disk (tengah, lebih rendah) ──────────
        {
            std::vector<float> v;
            float r=1.5f, h=0.18f;
            for (int i=0;i<SEG;i++) {
                float a0=(float)i/SEG*2*PI, a1=(float)(i+1)/SEG*2*PI;
                v.insert(v.end(),{0,h,0, 0,1,0, 0.5f,0.5f});
                v.insert(v.end(),{r*cosf(a0),h,r*sinf(a0), 0,1,0,
                                  0.5f+0.5f*cosf(a0),0.5f+0.5f*sinf(a0)});
                v.insert(v.end(),{r*cosf(a1),h,r*sinf(a1), 0,1,0,
                                  0.5f+0.5f*cosf(a1),0.5f+0.5f*sinf(a1)});
                // Wall
                v.insert(v.end(),{r*cosf(a0),0,r*sinf(a0), cosf(a0),0,sinf(a0),(float)i/SEG,0});
                v.insert(v.end(),{r*cosf(a0),h,r*sinf(a0), cosf(a0),0,sinf(a0),(float)i/SEG,1});
                v.insert(v.end(),{r*cosf(a1),h,r*sinf(a1), cosf(a1),0,sinf(a1),(float)(i+1)/SEG,1});
                v.insert(v.end(),{r*cosf(a1),h,r*sinf(a1), cosf(a1),0,sinf(a1),(float)(i+1)/SEG,1});
                v.insert(v.end(),{r*cosf(a1),0,r*sinf(a1), cosf(a1),0,sinf(a1),(float)(i+1)/SEG,0});
                v.insert(v.end(),{r*cosf(a0),0,r*sinf(a0), cosf(a0),0,sinf(a0),(float)i/SEG,0});
            }
            makeVAO(platInnerVAO, platInnerVBO, v, platInnerCount);
        }

        // ── 3. Panel segmen dekoratif (12 panel trapezoid) ─
        {
            std::vector<float> v;
            int   PANELS = 12;
            float rIn=1.52f, rOut=2.08f, hP=0.40f, hB=0.0f;
            float gap = 0.04f;

            for (int i=0;i<PANELS;i++) {
                float a0=(float)i/PANELS*2*PI + gap;
                float a1=(float)(i+1)/PANELS*2*PI - gap;
                float am=(a0+a1)*0.5f;
                float nx=cosf(am), nz=sinf(am);

                // Panel top face (sedikit lebih tinggi dari body)
                v.insert(v.end(),{rIn*cosf(a0),hP,rIn*sinf(a0),  0,1,0, 0,0});
                v.insert(v.end(),{rOut*cosf(a0),hP,rOut*sinf(a0), 0,1,0, 1,0});
                v.insert(v.end(),{rOut*cosf(a1),hP,rOut*sinf(a1), 0,1,0, 1,1});
                v.insert(v.end(),{rOut*cosf(a1),hP,rOut*sinf(a1), 0,1,0, 1,1});
                v.insert(v.end(),{rIn*cosf(a1),hP,rIn*sinf(a1),  0,1,0, 0,1});
                v.insert(v.end(),{rIn*cosf(a0),hP,rIn*sinf(a0),  0,1,0, 0,0});

                // Outer face panel
                v.insert(v.end(),{rOut*cosf(a0),hB,rOut*sinf(a0), nx,0,nz, 0,0});
                v.insert(v.end(),{rOut*cosf(a0),hP,rOut*sinf(a0), nx,0,nz, 0,1});
                v.insert(v.end(),{rOut*cosf(a1),hP,rOut*sinf(a1), nx,0,nz, 1,1});
                v.insert(v.end(),{rOut*cosf(a1),hP,rOut*sinf(a1), nx,0,nz, 1,1});
                v.insert(v.end(),{rOut*cosf(a1),hB,rOut*sinf(a1), nx,0,nz, 1,0});
                v.insert(v.end(),{rOut*cosf(a0),hB,rOut*sinf(a0), nx,0,nz, 0,0});

                // Side kiri panel
                v.insert(v.end(),{rIn*cosf(a0),hB,rIn*sinf(a0),  -sinf(a0),0,cosf(a0), 0,0});
                v.insert(v.end(),{rIn*cosf(a0),hP,rIn*sinf(a0),  -sinf(a0),0,cosf(a0), 0,1});
                v.insert(v.end(),{rOut*cosf(a0),hP,rOut*sinf(a0),-sinf(a0),0,cosf(a0), 1,1});
                v.insert(v.end(),{rOut*cosf(a0),hP,rOut*sinf(a0),-sinf(a0),0,cosf(a0), 1,1});
                v.insert(v.end(),{rOut*cosf(a0),hB,rOut*sinf(a0),-sinf(a0),0,cosf(a0), 1,0});
                v.insert(v.end(),{rIn*cosf(a0),hB,rIn*sinf(a0),  -sinf(a0),0,cosf(a0), 0,0});

                // Side kanan panel
                v.insert(v.end(),{rIn*cosf(a1),hB,rIn*sinf(a1),  sinf(a1),0,-cosf(a1), 0,0});
                v.insert(v.end(),{rIn*cosf(a1),hP,rIn*sinf(a1),  sinf(a1),0,-cosf(a1), 0,1});
                v.insert(v.end(),{rOut*cosf(a1),hP,rOut*sinf(a1), sinf(a1),0,-cosf(a1), 1,1});
                v.insert(v.end(),{rOut*cosf(a1),hP,rOut*sinf(a1), sinf(a1),0,-cosf(a1), 1,1});
                v.insert(v.end(),{rOut*cosf(a1),hB,rOut*sinf(a1), sinf(a1),0,-cosf(a1), 1,0});
                v.insert(v.end(),{rIn*cosf(a1),hB,rIn*sinf(a1),  sinf(a1),0,-cosf(a1), 0,0});
            }
            makeVAO(platPanelVAO, platPanelVBO, v, platPanelCount);
        }

        // ── 4. Gold/accent strip emissive di celah panel ──
        {
            std::vector<float> v;
            int   PANELS = 12;
            float rMid   = 1.78f;
            float thick  = 0.03f;
            float hStrip = 0.41f;
            float gap    = 0.04f;

            for (int i=0;i<PANELS;i++) {
                float a0=(float)i/PANELS*2*PI + gap*0.2f;
                float a1=(float)(i+1)/PANELS*2*PI - gap*0.2f;
                float ri=rMid-thick, ro=rMid+thick;

                v.insert(v.end(),{ri*cosf(a0),hStrip,ri*sinf(a0)});
                v.insert(v.end(),{ro*cosf(a0),hStrip,ro*sinf(a0)});
                v.insert(v.end(),{ro*cosf(a1),hStrip,ro*sinf(a1)});
                v.insert(v.end(),{ro*cosf(a1),hStrip,ro*sinf(a1)});
                v.insert(v.end(),{ri*cosf(a1),hStrip,ri*sinf(a1)});
                v.insert(v.end(),{ri*cosf(a0),hStrip,ri*sinf(a0)});
            }
            makeVAO(platGlowVAO, platGlowVBO, v, platGlowCount, false);
        }

        // ── 5. Step tangga kecil di belakang (seperti gambar)
        {
            std::vector<float> v;
            // 3 step, makin ke luar makin rendah
            float steps[3][3] = {
                // {rMin, rMax, height}
                {2.15f, 2.6f,  0.22f},
                {2.6f,  3.1f,  0.12f},
                {3.1f,  3.6f,  0.05f},
            };
            const int SSEG = 48;
            // Hanya bagian belakang (180 derajat)
            for (auto& s : steps) {
                for (int i=0;i<SSEG;i++) {
                    float a0 = (float)i/SSEG*PI + PI*0.5f;
                    float a1 = (float)(i+1)/SSEG*PI + PI*0.5f;
                    float u0=(float)i/SSEG, u1=(float)(i+1)/SSEG;
                    float rIn=s[0], rOut=s[1], h=s[2];
                    // Top
                    v.insert(v.end(),{rIn*cosf(a0),h,rIn*sinf(a0),  0,1,0, u0,0});
                    v.insert(v.end(),{rOut*cosf(a0),h,rOut*sinf(a0), 0,1,0, u0,1});
                    v.insert(v.end(),{rOut*cosf(a1),h,rOut*sinf(a1), 0,1,0, u1,1});
                    v.insert(v.end(),{rOut*cosf(a1),h,rOut*sinf(a1), 0,1,0, u1,1});
                    v.insert(v.end(),{rIn*cosf(a1),h,rIn*sinf(a1),  0,1,0, u1,0});
                    v.insert(v.end(),{rIn*cosf(a0),h,rIn*sinf(a0),  0,1,0, u0,0});
                }
            }
            makeVAO(platStepVAO, platStepVBO, v, platStepCount);
        }
    }

    void drawPlatforms(Shader& shader) {
        for (int i=0;i<3;i++) {
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, stagePos[i]);
            shader.setMat4("model", m);

            // Body ring utama — metalik gelap
            bindSolidTex(shader, platDarkTex);
            glBindVertexArray(platBodyVAO);
            glDrawArrays(GL_TRIANGLES,0,platBodyCount);

            // Inner disk — lebih gelap
            bindSolidTex(shader, platInnerTex);
            glBindVertexArray(platInnerVAO);
            glDrawArrays(GL_TRIANGLES,0,platInnerCount);

            // Panel segmen — sedikit berbeda
            bindSolidTex(shader, platPanelTex);
            glBindVertexArray(platPanelVAO);
            glDrawArrays(GL_TRIANGLES,0,platPanelCount);

            // Step tangga
            bindSolidTex(shader, platDarkTex);
            glBindVertexArray(platStepVAO);
            glDrawArrays(GL_TRIANGLES,0,platStepCount);

            glBindVertexArray(0);
        }
    }

    void drawPlatformGlow(Shader& emissiveShader) {
        glm::vec3 glowColors[3] = {
            {0.2f, 0.6f, 1.0f},   // biru
            {0.2f, 0.6f, 1.0f},   // biru
            {1.0f, 0.85f, 0.3f},  // emas
        };
        for (int i=0;i<3;i++) {
            glm::mat4 m = glm::mat4(1.0f);
            m = glm::translate(m, stagePos[i]);
            emissiveShader.setMat4("model", m);
            emissiveShader.setVec3("emissiveColor", glowColors[i]);
            emissiveShader.setFloat("emissiveStrength", 2.5f);
            glBindVertexArray(platGlowVAO);
            glDrawArrays(GL_TRIANGLES,0,platGlowCount);
            glBindVertexArray(0);
        }
    }

    // ── Neon Lines ───────────────────────────────────────
// Tambah di private member:

void setupNeonLines() {
    std::vector<float> verts;
    float hw = 13.9f, hd = 17.9f, H = 7.9f;
    float W  = 0.055f;

    auto addStrip = [&](glm::vec3 a, glm::vec3 b, float halfW) {
        glm::vec3 dir  = glm::normalize(b - a);
        glm::vec3 up   = (fabsf(dir.y) < 0.9f) ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
        glm::vec3 perp = glm::normalize(glm::cross(dir, up)) * halfW;
        glm::vec3 a0=a-perp, a1=a+perp, b0=b-perp, b1=b+perp;
        auto push = [&](glm::vec3 v){
            verts.insert(verts.end(),{v.x,v.y,v.z});
        };
        push(a0); push(a1); push(b1);
        push(b1); push(b0); push(a0);
    };

    // ════════════════════════════════════════════════════
    // LANGIT-LANGIT — grid rectangular seperti referensi
    // False ceiling tiles dengan light strips di gap antar tile
    // ════════════════════════════════════════════════════

    // Grid utama langit-langit: 4 baris × 3 kolom tile
    // Tile size ~6x6 unit, gap ~0.2 antar tile
    // Strip horizontal (melintang Z)
    float ceilStrips_Z[] = {-hd, -10.f, -4.f, 2.f, 8.f, hd};
    for (float z : ceilStrips_Z) {
        addStrip({-hw, H, z}, {hw, H, z}, W);
    }

    // Strip vertikal (melintang X)  
    float ceilStrips_X[] = {-8.f, -2.f, 4.f, 10.f};
    for (float x : ceilStrips_X) {
        addStrip({x, H, -hd}, {x, H, hd}, W);
    }

    // ── Accent: beberapa tile punya inner frame ──────────
    // Tile kiri-depan inner frame
    addStrip({-hw, H, -hd},  {-8.f, H, -hd},  W); // udah dari grid
    // Inner box tile [kiri-depan]
    addStrip({-12.f,H,-16.f},{-9.f,H,-16.f},  W*0.8f);
    addStrip({-12.f,H,-16.f},{-12.f,H,-12.f}, W*0.8f);

    // Inner box tile [kanan-tengah]  
    addStrip({5.f,H,-3.f},   {9.f,H,-3.f},    W*0.8f);
    addStrip({5.f,H,-3.f},   {5.f,H,1.f},     W*0.8f);
    addStrip({9.f,H,-3.f},   {9.f,H,1.f},     W*0.8f);
    addStrip({5.f,H,1.f},    {9.f,H,1.f},     W*0.8f);

    // Inner box tile [tengah-belakang]
    addStrip({-3.f,H,3.f},   {2.f,H,3.f},     W*0.8f);
    addStrip({-3.f,H,3.f},   {-3.f,H,8.f},    W*0.8f);
    addStrip({2.f,H,3.f},    {2.f,H,8.f},     W*0.8f);
    addStrip({-3.f,H,8.f},   {2.f,H,8.f},     W*0.8f);

    // ── Drop lines: langit → dinding (kayak referensi) ───
    // Beberapa grid lines turun ke dinding
    addStrip({-8.f,H,-hd},  {-8.f,4.5f,-hd},  W);   // kiri → dinding depan
    addStrip({4.f,H,-hd},   {4.f,4.0f,-hd},   W);   // kanan → dinding depan
    addStrip({-2.f,H,-hd},  {-2.f,5.5f,-hd},  W);   // tengah
    addStrip({-8.f,H,hd},   {-8.f,3.5f,hd},   W);   // → dinding belakang
    addStrip({10.f,H,hd},   {10.f,4.5f,hd},   W);
    addStrip({-hw,H,-4.f},  {-hw,4.0f,-4.f},  W);   // → dinding kiri
    addStrip({-hw,H, 8.f},  {-hw,3.5f, 8.f},  W);
    addStrip({hw,H,-10.f},  {hw,4.5f,-10.f},  W);   // → dinding kanan
    addStrip({hw,H, 2.f},   {hw,3.5f, 2.f},   W);

    // ════════════════════════════════════════════════════
    // DINDING DEPAN (-Z) — frame besar + detail
    // Referensi: frame kotak prominent + garis horizontal
    // ════════════════════════════════════════════════════

    // Frame besar kiri dinding depan
    addStrip({-hw,  0.5f,-hd},{-hw,  7.2f,-hd}, W*1.2f); // tepi kiri (vertikal)
    addStrip({-hw,  7.2f,-hd},{-3.f, 7.2f,-hd}, W*1.2f); // atas
    addStrip({-3.f, 7.2f,-hd},{-3.f, 0.5f,-hd}, W*1.2f); // kanan
    // (bawah ketemu lantai, skip)

    // Frame besar kanan dinding depan  
    addStrip({hw,   0.5f,-hd},{hw,   7.2f,-hd}, W*1.2f);
    addStrip({hw,   7.2f,-hd},{4.f,  7.2f,-hd}, W*1.2f);
    addStrip({4.f,  7.2f,-hd},{4.f,  0.5f,-hd}, W*1.2f);

    // Strip horizontal mid-wall
    addStrip({-hw,  4.0f,-hd},{hw,   4.0f,-hd}, W);      // garis tengah penuh
    addStrip({-hw,  2.0f,-hd},{-3.f, 2.0f,-hd}, W*0.8f); // sub kiri bawah
    addStrip({4.f,  2.0f,-hd},{hw,   2.0f,-hd}, W*0.8f); // sub kanan bawah

    // Inner frame kecil di tengah (accent)
    addStrip({-1.f, 3.5f,-hd},{2.f,  3.5f,-hd}, W*0.7f);
    addStrip({-1.f, 3.5f,-hd},{-1.f, 5.5f,-hd}, W*0.7f);
    addStrip({2.f,  3.5f,-hd},{2.f,  5.5f,-hd}, W*0.7f);
    addStrip({-1.f, 5.5f,-hd},{2.f,  5.5f,-hd}, W*0.7f);

    // ════════════════════════════════════════════════════
    // DINDING KIRI (-X)
    // ════════════════════════════════════════════════════

    // Frame besar (full height, partial width)
    addStrip({-hw, 0.5f,-10.f},{-hw,7.2f,-10.f}, W*1.2f);
    addStrip({-hw, 7.2f,-10.f},{-hw,7.2f,  2.f}, W*1.2f);
    addStrip({-hw, 7.2f,  2.f},{-hw,0.5f,  2.f}, W*1.2f);
    // Inner horizontal
    addStrip({-hw, 3.5f,-10.f},{-hw,3.5f,  2.f}, W);

    // Frame kecil kanan-kiri
    addStrip({-hw, 1.0f, 6.f},{-hw,5.0f, 6.f},  W);
    addStrip({-hw, 5.0f, 6.f},{-hw,5.0f,12.f},  W);
    addStrip({-hw, 5.0f,12.f},{-hw,1.0f,12.f},  W);
    addStrip({-hw, 1.0f,12.f},{-hw,1.0f, 6.f},  W);
    // Inner cross
    addStrip({-hw, 3.0f, 6.f},{-hw,3.0f,12.f},  W*0.7f);

    // Strip pendek accent kiri atas
    addStrip({-hw, 6.0f,-16.f},{-hw,6.0f,-12.f},W*0.8f);
    addStrip({-hw, 6.0f,-16.f},{-hw,3.0f,-16.f},W*0.8f);

    // ════════════════════════════════════════════════════
    // DINDING KANAN (+X)
    // ════════════════════════════════════════════════════

    // Frame besar
    addStrip({hw, 0.5f,-8.f},{hw,7.2f,-8.f},  W*1.2f);
    addStrip({hw, 7.2f,-8.f},{hw,7.2f, 6.f},  W*1.2f);
    addStrip({hw, 7.2f, 6.f},{hw,0.5f, 6.f},  W*1.2f);
    addStrip({hw, 0.5f, 6.f},{hw,0.5f,-8.f},  W*1.2f);

    // Inner detail dalam frame besar
    addStrip({hw, 4.0f,-8.f},{hw,4.0f, 6.f},  W);      // horizontal tengah
    addStrip({hw, 0.5f,-1.f},{hw,7.2f,-1.f},  W*0.8f); // vertikal tengah

    // Frame kecil kanan
    addStrip({hw, 1.5f, 9.f},{hw,5.5f, 9.f},  W);
    addStrip({hw, 5.5f, 9.f},{hw,5.5f,14.f},  W);
    addStrip({hw, 5.5f,14.f},{hw,1.5f,14.f},  W);
    addStrip({hw, 1.5f,14.f},{hw,1.5f, 9.f},  W);

    // Accent strip
    addStrip({hw, 2.5f,-14.f},{hw,2.5f,-10.f},W*0.8f);
    addStrip({hw, 6.0f,-14.f},{hw,6.0f,-10.f},W*0.8f);
    addStrip({hw, 6.0f,-14.f},{hw,2.5f,-14.f},W*0.8f);

    // ════════════════════════════════════════════════════
    // DINDING BELAKANG (+Z)
    // ════════════════════════════════════════════════════

    // Frame penuh seperti referensi bagian belakang
    addStrip({-hw,0.5f,hd},{-hw,7.0f,hd},  W*1.2f);
    addStrip({-hw,7.0f,hd},{ hw,7.0f,hd},  W*1.2f);
    addStrip({ hw,7.0f,hd},{ hw,0.5f,hd},  W*1.2f);

    // Inner
    addStrip({-hw,4.0f,hd},{hw,4.0f,hd},   W);
    addStrip({-5.f,0.5f,hd},{-5.f,4.0f,hd},W*0.8f);
    addStrip({ 5.f,0.5f,hd},{ 5.f,7.0f,hd},W*0.8f);
    addStrip({-5.f,4.0f,hd},{ 5.f,4.0f,hd},W*0.7f);

    neonQuadCount = (int)(verts.size() / 3);
    glGenVertexArrays(1,&neonVAO); glGenBuffers(1,&neonVBO);
    glBindVertexArray(neonVAO);
    glBindBuffer(GL_ARRAY_BUFFER,neonVBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(float), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
    glBindVertexArray(0);
}
void drawNeonLines(Shader& s) {
    s.setMat4("model", glm::mat4(1.0f));
    s.setVec3("emissiveColor",    glm::vec3(1.0f));
    s.setFloat("emissiveStrength", 4.5f);  // terang banget biar bloom nangkep
    glBindVertexArray(neonVAO);
    glDrawArrays(GL_TRIANGLES, 0, neonQuadCount);
    glBindVertexArray(0);
}

    // ── Glow Ring ────────────────────────────────────────
    void setupGlowRings() {
        const int SEG=128;
        std::vector<float> v;
        float r=2.25f, thick=0.05f;
        for (int i=0;i<SEG;i++) {
            float a0=(float)i/SEG*2*PI, a1=(float)(i+1)/SEG*2*PI;
            float ri=r-thick, ro=r+thick;
            v.insert(v.end(),{ri*cosf(a0),0.01f,ri*sinf(a0)});
            v.insert(v.end(),{ro*cosf(a0),0.01f,ro*sinf(a0)});
            v.insert(v.end(),{ro*cosf(a1),0.01f,ro*sinf(a1)});
            v.insert(v.end(),{ro*cosf(a1),0.01f,ro*sinf(a1)});
            v.insert(v.end(),{ri*cosf(a1),0.01f,ri*sinf(a1)});
            v.insert(v.end(),{ri*cosf(a0),0.01f,ri*sinf(a0)});
        }
        ringVertCount=(int)(v.size()/3);
        glGenVertexArrays(1,&ringVAO); glGenBuffers(1,&ringVBO);
        glBindVertexArray(ringVAO);
        glBindBuffer(GL_ARRAY_BUFFER,ringVBO);
        glBufferData(GL_ARRAY_BUFFER,v.size()*sizeof(float),v.data(),GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
        glBindVertexArray(0);
    }

    void drawGlowRings(Shader& s) {
        glm::vec3 colors[3]={{0.2f,0.6f,1.0f},{0.2f,0.6f,1.0f},{1.0f,0.85f,0.3f}};
        for (int i=0;i<3;i++) {
            glm::mat4 m=glm::mat4(1.0f);
            m=glm::translate(m,stagePos[i]);
            s.setMat4("model",m);
            s.setVec3("emissiveColor",colors[i]);
            s.setFloat("emissiveStrength",4.0f);
            glBindVertexArray(ringVAO);
            glDrawArrays(GL_TRIANGLES,0,ringVertCount);
            glBindVertexArray(0);
        }
    }

    // ── Skybox ───────────────────────────────────────────
    void setupSkybox() {
        float v[]={
            -1,1,-1,-1,-1,-1,1,-1,-1,1,-1,-1,1,1,-1,-1,1,-1,
            -1,-1,1,-1,-1,-1,-1,1,-1,-1,1,-1,-1,1,1,-1,-1,1,
            1,-1,-1,1,-1,1,1,1,1,1,1,1,1,1,-1,1,-1,-1,
            -1,-1,1,-1,1,1,1,1,1,1,1,1,1,-1,1,-1,-1,1,
            -1,1,-1,1,1,-1,1,1,1,1,1,1,-1,1,1,-1,1,-1,
            -1,-1,-1,-1,-1,1,1,-1,-1,1,-1,-1,-1,-1,1,1,-1,1
        };
        glGenVertexArrays(1,&skyboxVAO); glGenBuffers(1,&skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER,skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER,sizeof(v),v,GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,3*sizeof(float),(void*)0);
        cubemapTexture=loadSolidCubemap(5,7,15);
    }

    // ── Utilities ────────────────────────────────────────
    void bindSolidTex(Shader& shader, unsigned int tex) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D,tex);
        shader.setInt("texture_diffuse1",0);
    }

    unsigned int loadSolidColor(int r,int g,int b) {
        unsigned char d[3]={(unsigned char)r,(unsigned char)g,(unsigned char)b};
        unsigned int id; glGenTextures(1,&id);
        glBindTexture(GL_TEXTURE_2D,id);
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,1,1,0,GL_RGB,GL_UNSIGNED_BYTE,d);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        return id;
    }

    unsigned int loadSolidCubemap(int r,int g,int b) {
        unsigned char d[3]={(unsigned char)r,(unsigned char)g,(unsigned char)b};
        unsigned int id; glGenTextures(1,&id);
        glBindTexture(GL_TEXTURE_CUBE_MAP,id);
        for(int i=0;i<6;i++)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB,1,1,0,GL_RGB,GL_UNSIGNED_BYTE,d);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
        return id;
    }
};