#pragma once

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

class Model {
public:
    Model(const std::string& modelPath, const std::string& texturePath);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    void draw() const;

private:
    struct Mesh {
        GLuint vao = 0;
        GLuint vbo = 0;
        GLuint ebo = 0;
        GLsizei indexCount = 0;
    };

    std::vector<Mesh> meshes;
    GLuint texture = 0;

    void loadTexture(const std::string& path);
    void release();
};