#pragma once
#include <glad/glad.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertPath, const char* fragPath, const char* geomPath = nullptr) {
        std::string vCode, fCode, gCode;
        auto readFile = [](const char* path) {
            std::ifstream f(path);
            std::stringstream ss;
            ss << f.rdbuf();
            return ss.str();
        };
        vCode = readFile(vertPath);
        fCode = readFile(fragPath);
        if (geomPath) gCode = readFile(geomPath);

        auto compile = [](const std::string& src, GLenum type) {
            const char* c = src.c_str();
            unsigned int s = glCreateShader(type);
            glShaderSource(s, 1, &c, nullptr);
            glCompileShader(s);
            int ok; char log[512];
            glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
            if (!ok) { glGetShaderInfoLog(s, 512, nullptr, log); std::cerr << "SHADER ERR:\n" << log << "\n"; }
            return s;
        };

        unsigned int v = compile(vCode, GL_VERTEX_SHADER);
        unsigned int f = compile(fCode, GL_FRAGMENT_SHADER);
        unsigned int g = geomPath ? compile(gCode, GL_GEOMETRY_SHADER) : 0;

        ID = glCreateProgram();
        glAttachShader(ID, v);
        glAttachShader(ID, f);
        if (g) glAttachShader(ID, g);
        glLinkProgram(ID);
        int ok; char log[512];
        glGetProgramiv(ID, GL_LINK_STATUS, &ok);
        if (!ok) { glGetProgramInfoLog(ID, 512, nullptr, log); std::cerr << "PROGRAM ERR:\n" << log << "\n"; }

        glDeleteShader(v);
        glDeleteShader(f);
        if (g) glDeleteShader(g);
    }

    Shader& use() { glUseProgram(ID); return *this; }
    void setBool (const std::string& n, bool v)              const { glUniform1i(loc(n), (int)v); }
    void setInt  (const std::string& n, int v)               const { glUniform1i(loc(n), v); }
    void setFloat(const std::string& n, float v)             const { glUniform1f(loc(n), v); }
    void setVec2 (const std::string& n, glm::vec2 v)         const { glUniform2fv(loc(n), 1, glm::value_ptr(v)); }
    void setVec3 (const std::string& n, glm::vec3 v)         const { glUniform3fv(loc(n), 1, glm::value_ptr(v)); }
    void setVec3 (const std::string& n, float x, float y, float z) const { glUniform3f(loc(n), x, y, z); }
    void setVec4 (const std::string& n, glm::vec4 v)         const { glUniform4fv(loc(n), 1, glm::value_ptr(v)); }
    void setMat4 (const std::string& n, glm::mat4 v)         const { glUniformMatrix4fv(loc(n), 1, GL_FALSE, glm::value_ptr(v)); }

private:
    int loc(const std::string& n) const { return glGetUniformLocation(ID, n.c_str()); }
};