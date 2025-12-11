#ifndef SOLARSYSTEM_MESHKEEPER_H
#define SOLARSYSTEM_MESHKEEPER_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 textureCoords;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

struct Texture {
    size_t id;
    std::string type;
    aiString path;
};

struct Mesh{
    std::vector<Vertex> _vertices; // Вершины
    std::vector<size_t> _indices; // Индексы
    std::vector<Texture> _textures; // Текстуры
};

size_t TextureFromFile(const std::string& path); // Not used at all

class MeshHolder {
public:
    explicit MeshHolder(const std::string& path);

    const std::vector<Texture>& getLoadedTextures() const;
    const std::vector<Mesh>& getMeshes()  const;
private:

    void ProcessNode(aiNode* node, const aiScene* scene);
    Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> LoadMaterialTextures(aiMaterial* mat, const aiTextureType& type, const std::string& typeName);
private:
    std::vector<Texture> _loadedTextures;
    std::vector<Mesh> _meshes;
    std::string _directory;

};

#endif //SOLARSYSTEM_MESHKEEPER_H
