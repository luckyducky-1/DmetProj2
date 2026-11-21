#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>
#include <assimp/material.h>
#include <cstddef>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <limits>
#include <filesystem>
#include <unordered_map>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace {
    struct Vertex {
        float position[3];
        float normal[3];
        float uv[2];
    };
}

Model::Model(
    const std::string& modelPath,
    const std::string& texturePath
)
{
    Assimp::Importer importer;

    // Bake imported node transforms into the mesh vertices.
    unsigned int importFlags =
    aiProcess_Triangulate |
    aiProcess_GenSmoothNormals;

bool isChest =
    modelPath.find("treasure_chest_1k.gltf") != std::string::npos;

bool isGlobe =
    modelPath.find("globe_separated.glb") != std::string::npos;

if (!isChest && !isGlobe) {
    importFlags |= aiProcess_PreTransformVertices;
}
const aiScene* scene = importer.ReadFile(
    modelPath,
    importFlags
);
if (scene &&
    modelPath.find("Old_Antique_Standing_Globe_") != std::string::npos) {

    std::cout << "\nGlobe nodes:\n";

    auto printNodes = [&](auto&& self, const aiNode* node, int depth) -> void {
        std::cout << std::string(depth * 2, ' ')
                  << node->mName.C_Str()
                  << " | Meshes: " << node->mNumMeshes
                  << std::endl;

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            self(self, node->mChildren[i], depth + 1);
        }
    };

    printNodes(printNodes, scene->mRootNode, 0);
}
    if (!scene || !scene->mRootNode || !scene->HasMeshes()) {
        throw std::runtime_error(
            "Cannot load model: " + modelPath +
            "\n" + importer.GetErrorString()
        );
    }

    try {

        // Measure the whole model before uploading its meshes.
        // Keep track of each chest part's original position
std::vector<aiMatrix4x4> meshTransforms(
    scene->mNumMeshes,
    aiMatrix4x4()
);

if (isChest || isGlobe) {
    auto applyNodeTransforms =
        [&](auto&& self, const aiNode* node,
            const aiMatrix4x4& parentTransform) -> void {

        aiMatrix4x4 worldTransform =
            parentTransform * node->mTransformation;

        for (unsigned int j = 0; j < node->mNumMeshes; ++j) {
            unsigned int meshIndex = node->mMeshes[j];
            meshTransforms[meshIndex] = worldTransform;
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i) {
            self(self, node->mChildren[i], worldTransform);
        }
    };

    applyNodeTransforms(
        applyNodeTransforms,
        scene->mRootNode,
        aiMatrix4x4()
    );
}
float largest = std::numeric_limits<float>::max();

aiVector3D minimum(largest, largest, largest);
aiVector3D maximum(-largest, -largest, -largest);

for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
    const aiMesh* mesh = scene->mMeshes[m];

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
       aiVector3D p = meshTransforms[m] * mesh->mVertices[i];

        minimum.x = std::min(minimum.x, p.x);
        minimum.y = std::min(minimum.y, p.y);
        minimum.z = std::min(minimum.z, p.z);

        maximum.x = std::max(maximum.x, p.x);
        maximum.y = std::max(maximum.y, p.y);
        maximum.z = std::max(maximum.z, p.z);
    }
}

aiVector3D center = (minimum + maximum) * 0.5f;
aiVector3D size = maximum - minimum;

float longestSide = std::max({size.x, size.y, size.z});

if (longestSide <= 0.0f) {
    throw std::runtime_error("Model has no measurable size.");
}


float fitScale = 2.0f / longestSide;
// Store the bottom after centering and scaling
bottom = (minimum.y - center.y) * fitScale;
        meshes.reserve(scene->mNumMeshes);
std::unordered_map<std::string, GLuint> loadedTextures;

std::filesystem::path modelFolder =
    std::filesystem::path(modelPath).parent_path();
        for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
            const aiMesh* source = scene->mMeshes[m];
aiString materialName;

scene->mMaterials[source->mMaterialIndex]->Get(
    AI_MATKEY_NAME, materialName
);

// Skip the glass so the clock face stays visible
if (std::string(materialName.C_Str()) ==
    "vintage_grandfather_clock_01_glass") {
    continue;
}
            if (!source->HasPositions() ||
                !source->HasNormals() ||
                !source->HasTextureCoords(0)) {
                throw std::runtime_error(
                    "A mesh is missing positions, normals, or texture coordinates."
                );
            }

            std::vector<Vertex> vertices;
            std::vector<unsigned int> indices;

            vertices.reserve(source->mNumVertices);

            for (unsigned int i = 0; i < source->mNumVertices; ++i) {
               aiVector3D p = meshTransforms[m] * source->mVertices[i];

aiMatrix3x3 normalTransform(meshTransforms[m]);
normalTransform.Inverse().Transpose();

aiVector3D n = normalTransform * source->mNormals[i];
n.Normalize();

const auto& uv = source->mTextureCoords[0][i];
             vertices.push_back({
    {
        (p.x - center.x) * fitScale,
        (p.y - center.y) * fitScale,
        (p.z - center.z) * fitScale
    },
    {n.x, n.y, n.z},
    {uv.x, uv.y}
});
            }

            for (unsigned int i = 0; i < source->mNumFaces; ++i) {
                const aiFace& face = source->mFaces[i];

                if (face.mNumIndices != 3) {
                    continue;
                }

                for (unsigned int j = 0; j < 3; ++j) {
                    indices.push_back(face.mIndices[j]);
                }
            }

            meshes.emplace_back();
            Mesh& mesh = meshes.back();
            mesh.name = source->mName.C_Str();
      if (isGlobe && m == 1) {
    float sphereMin[3] = {
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    };

    float sphereMax[3] = {
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::lowest()
    };

    for (const auto& vertex : vertices) {
        for (int i = 0; i < 3; ++i) {
            sphereMin[i] = std::min(sphereMin[i], vertex.position[i]);
            sphereMax[i] = std::max(sphereMax[i], vertex.position[i]);
        }
    }

    std::cout << "Sphere center: "
              << (sphereMin[0] + sphereMax[0]) * 0.5f << ", "
              << (sphereMin[1] + sphereMax[1]) * 0.5f << ", "
              << (sphereMin[2] + sphereMax[2]) * 0.5f
              << std::endl;
}


            // Show the globe parts so we can find the sphere
// Check the parts of the new globe model
if (modelPath.find("globe_separated.glb") != std::string::npos) {
    std::cout << "Globe mesh " << m
              << " | Name: " << mesh.name
              << " | Vertices: " << source->mNumVertices
              << std::endl;
}
            mesh.indexCount = static_cast<GLsizei>(indices.size());
  if (modelPath.find("treasure_chest") != std::string::npos) {
    std::cout << "Mesh index: " << m
              << " | Name: " << mesh.name
              << " | Vertices: " << source->mNumVertices
              << std::endl;
}
            if (modelPath.find("treasure_chest") != std::string::npos) {
    std::cout << "Chest mesh: " << mesh.name << std::endl;
}
// Use the mesh's own colour texture when one is available
const aiMaterial* material =
    scene->mMaterials[source->mMaterialIndex];

aiString imagePath;

bool hasImage =
    material->GetTexture(aiTextureType_BASE_COLOR, 0, &imagePath)
        == AI_SUCCESS;

if (!hasImage) {
    hasImage =
        material->GetTexture(aiTextureType_DIFFUSE, 0, &imagePath)
            == AI_SUCCESS;
}

if (hasImage) {
    std::string relativePath = imagePath.C_Str();
    std::replace(relativePath.begin(), relativePath.end(), '\\', '/');

    std::filesystem::path fullPath = modelFolder / relativePath;

    if (std::filesystem::is_regular_file(fullPath)) {
        std::string key = fullPath.lexically_normal().string();
        auto existing = loadedTextures.find(key);

        if (existing != loadedTextures.end()) {
            mesh.texture = existing->second;
        } else {
            loadTexture(key);
            mesh.texture = texture;
            materialTextures.push_back(texture);
            texture = 0;
            loadedTextures.emplace(key, mesh.texture);
        }
    }
}
            glGenVertexArrays(1, &mesh.vao);
            glGenBuffers(1, &mesh.vbo);
            glGenBuffers(1, &mesh.ebo);

            glBindVertexArray(mesh.vao);

            glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo);
            glBufferData(
                GL_ARRAY_BUFFER,
                vertices.size() * sizeof(Vertex),
                vertices.data(),
                GL_STATIC_DRAW
            );

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.ebo);
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER,
                indices.size() * sizeof(unsigned int),
                indices.data(),
                GL_STATIC_DRAW
            );

            glVertexAttribPointer(
                0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                reinterpret_cast<const void*>(offsetof(Vertex, position))
            );
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(
                1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                reinterpret_cast<const void*>(offsetof(Vertex, normal))
            );
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(
                2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                reinterpret_cast<const void*>(offsetof(Vertex, uv))
            );
            glEnableVertexAttribArray(2);

            glBindVertexArray(0);
        }

        loadTexture(texturePath);
    } catch (...) {
        release();
        throw;
    }
}

void Model::loadTexture(const std::string& path)
{
    int width, height, channels;

    stbi_set_flip_vertically_on_load(true);
    unsigned char* pixels = stbi_load(
        path.c_str(), &width, &height, &channels, STBI_rgb_alpha
    );

    if (!pixels) {
        throw std::runtime_error("Cannot load texture: " + path);
    }

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR
    );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(
        GL_TEXTURE_2D, 0, GL_RGBA8,
        width, height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, pixels
    );
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(pixels);
}

void Model::draw() const
{
    glActiveTexture(GL_TEXTURE0);

    for (const Mesh& mesh : meshes) {
        GLuint meshTexture = mesh.texture != 0
            ? mesh.texture
            : texture;

        glBindTexture(GL_TEXTURE_2D, meshTexture);
        glBindVertexArray(mesh.vao);

        glDrawElements(
            GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr
        );
    }

    glBindVertexArray(0);
}
void Model::drawMesh(unsigned int index) const
{
    if (index >= meshes.size()) {
        return;
    }

    const Mesh& mesh = meshes[index];

    GLuint meshTexture = mesh.texture != 0
        ? mesh.texture
        : texture;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, meshTexture);
    glBindVertexArray(mesh.vao);

    glDrawElements(
        GL_TRIANGLES,
        mesh.indexCount,
        GL_UNSIGNED_INT,
        nullptr
    );

    glBindVertexArray(0);
}
void Model::release()
{
    for (const Mesh& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.vao);
        glDeleteBuffers(1, &mesh.vbo);
        glDeleteBuffers(1, &mesh.ebo);
    }
for (GLuint image : materialTextures) {
    glDeleteTextures(1, &image);
}

materialTextures.clear();
    meshes.clear();
    glDeleteTextures(1, &texture);
    texture = 0;
}

Model::~Model()
{
    release();
}

float Model::getBottom() const
{
    return bottom;
}
