#pragma once

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

class Model {
public:
    Model(
        const std::string& modelPath,
        const std::string& texturePath
    );

    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    void draw() const;
    void drawMesh(unsigned int index) const;
float getBottom() const;
private:
float bottom = 0.0f;
    struct Mesh {
        std::string name;
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        GLuint texture = 0;
        GLsizei indexCount = 0;
        
    };

    std::vector<Mesh> meshes;

    // Used when a mesh has no texture of its own
    GLuint texture = 0;

    // Textures loaded from the model's materials
    std::vector<GLuint> materialTextures;

    void loadTexture(const std::string& path);
    void release();
};