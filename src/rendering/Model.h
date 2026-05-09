#pragma once
#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stb_image.h>
#include "Mesh.h"
#include <iostream>
#include <map>

unsigned int TextureFromFile(const char* path, const std::string& dir);

class Model {
public:
    std::vector<Texture> textures_loaded;
    std::vector<Mesh>    meshes;
    std::string          directory;

    Model(const std::string& path) { loadModel(path); }

    void Draw(Shader& shader) {
        for (auto& m : meshes) m.Draw(shader);
    }

private:
    void loadModel(const std::string& path) {
        Assimp::Importer imp;
        const aiScene* scene = imp.ReadFile(path,
            aiProcess_Triangulate | aiProcess_GenSmoothNormals |
            aiProcess_FlipUVs     | aiProcess_CalcTangentSpace);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ASSIMP: " << imp.GetErrorString() << "\n"; return;
        }
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);
    }

    void processNode(aiNode* node, const aiScene* scene) {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
            meshes.push_back(processMesh(scene->mMeshes[node->mMeshes[i]], scene));
        for (unsigned int i = 0; i < node->mNumChildren; i++)
            processNode(node->mChildren[i], scene);
    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture>      textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Vertex v;
            v.Position  = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            v.Normal    = mesh->HasNormals()
                        ? glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z)
                        : glm::vec3(0.0f);
            if (mesh->mTextureCoords[0]) {
                v.TexCoords = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                v.Tangent   = { mesh->mTangents[i].x,   mesh->mTangents[i].y,   mesh->mTangents[i].z };
                v.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
            } else { v.TexCoords = glm::vec2(0.0f); }
            vertices.push_back(v);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
            for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
                indices.push_back(mesh->mFaces[i].mIndices[j]);

        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        auto load = [&](aiTextureType type, const std::string& typeName) {
            std::vector<Texture> maps;
            for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
                aiString str; mat->GetTexture(type, i, &str);
                bool skip = false;
                for (auto& tl : textures_loaded)
                    if (tl.path == str.C_Str()) { maps.push_back(tl); skip = true; break; }
                if (!skip) {
                    Texture t;
                    t.id   = TextureFromFile(str.C_Str(), directory);
                    t.type = typeName;
                    t.path = str.C_Str();
                    maps.push_back(t);
                    textures_loaded.push_back(t);
                }
            }
            return maps;
        };

        auto diff = load(aiTextureType_DIFFUSE,  "texture_diffuse");
        textures.insert(textures.end(), diff.begin(), diff.end());

        auto spec = load(aiTextureType_SPECULAR, "texture_specular");
        if (spec.empty()) spec = load(aiTextureType_SHININESS, "texture_specular");
        textures.insert(textures.end(), spec.begin(), spec.end());

        auto norm = load(aiTextureType_NORMALS,  "texture_normal");
        if (norm.empty()) norm = load(aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), norm.begin(), norm.end());

        return Mesh(vertices, indices, textures);
    }
};

unsigned int TextureFromFile(const char* path, const std::string& dir) {
    std::string filename = dir + '/' + std::string(path);
    unsigned int id; glGenTextures(1, &id);
    int w, h, ch;
    unsigned char* data = stbi_load(filename.c_str(), &w, &h, &ch, 0);
    if (data) {
        GLenum fmt = ch==1 ? GL_RED : ch==3 ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        std::cout << "Loaded: " << filename << " (" << ch << "ch)\n";
    } else { std::cerr << "Failed: " << filename << "\n"; }
    stbi_image_free(data);
    return id;
}