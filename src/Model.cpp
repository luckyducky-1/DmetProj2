#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstddef>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include <limits>
#include <assimp/material.h>
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
    const aiScene* scene = importer.ReadFile(
        modelPath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_PreTransformVertices
    );

    if (!scene || !scene->mRootNode || !scene->HasMeshes()) {
        throw std::runtime_error(
            "Cannot load model: " + modelPath +
            "\n" + importer.GetErrorString()
        );
    }

    try {
        // Measure the whole model before uploading its meshes.
float largest = std::numeric_limits<float>::max();

aiVector3D minimum(largest, largest, largest);
aiVector3D maximum(-largest, -largest, -largest);

for (unsigned int m = 0; m < scene->mNumMeshes; ++m) {
    const aiMesh* mesh = scene->mMeshes[m];

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        const aiVector3D& p = mesh->mVertices[i];

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
        meshes.reserve(scene->mNumMeshes);

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
                const auto& p = source->mVertices[i];
                const auto& n = source->mNormals[i];
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
            mesh.indexCount = static_cast<GLsizei>(indices.size());

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
    glBindTexture(GL_TEXTURE_2D, texture);

    for (const Mesh& mesh : meshes) {
        glBindVertexArray(mesh.vao);
        glDrawElements(
            GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr
        );
    }

    glBindVertexArray(0);
}

void Model::release()
{
    for (const Mesh& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.vao);
        glDeleteBuffers(1, &mesh.vbo);
        glDeleteBuffers(1, &mesh.ebo);
    }

    meshes.clear();
    glDeleteTextures(1, &texture);
    texture = 0;
}

Model::~Model()
{
    release();
}