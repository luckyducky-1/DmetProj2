#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include "Model.h"
void resizeWindow(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void reportError(int code, const char* message)
{
    std::cerr << "GLFW error " << code << ": " << message << '\n';
}

std::string readFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error("Cannot open: " + path);
    }

    std::stringstream contents;
    contents << file.rdbuf();
    return contents.str();
}

GLuint compileShader(GLenum type, const std::string& path)
{
    std::string source = readFile(path);
    const char* sourcePointer = source.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &sourcePointer, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        char message[2048];
        glGetShaderInfoLog(shader, sizeof(message), nullptr, message);
        glDeleteShader(shader);
        throw std::runtime_error(path + "\n" + message);
    }

    return shader;
}

GLuint createShaderProgram()
{
    std::string root = PROJECT_ROOT;

    GLuint vertex = compileShader(
        GL_VERTEX_SHADER, root + "/shaders/scene.vert"
    );

    GLuint fragment;
    try {
        fragment = compileShader(
            GL_FRAGMENT_SHADER, root + "/shaders/scene.frag"
        );
    } catch (...) {
        glDeleteShader(vertex);
        throw;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success) {
        char message[2048];
        glGetProgramInfoLog(program, sizeof(message), nullptr, message);
        glDeleteProgram(program);
        throw std::runtime_error(std::string("Shader link failed: ") + message);
    }

    return program;
}

int main()
{
    glfwSetErrorCallback(reportError);

    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(
        1200, 800, "Haunted House - Study", nullptr, nullptr
    );

    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(window, resizeWindow);
    glEnable(GL_DEPTH_TEST);

    GLuint program = 0;
    int result = 0;

    try {
        program = createShaderProgram();
        glUseProgram(program);

        std::string folder =
            std::string(PROJECT_ROOT) +
            "/assets/models/painted_wooden_table_1k/";

        // Keep the model alive only while the OpenGL context exists.
        Model table(
            folder + "painted_wooden_table_1k.fbx",
            folder + "textures/painted_wooden_table_diff_1k.jpg"
        );

        GLint modelLocation = glGetUniformLocation(program, "model");
        GLint viewLocation = glGetUniformLocation(program, "view");
        GLint projectionLocation =
            glGetUniformLocation(program, "projection");

        glUniform1i(
            glGetUniformLocation(program, "diffuseTexture"), 0
        );

       glm::mat4 view = glm::lookAt(
    glm::vec3(3.0f, 2.2f, 4.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f)
);

        glUniformMatrix4fv(
            viewLocation, 1, GL_FALSE, glm::value_ptr(view)
        );

        glm::mat4 model(1.0f);

        glUniformMatrix4fv(
            modelLocation, 1, GL_FALSE, glm::value_ptr(model)
        );

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }

            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            if (width == 0 || height == 0) {
                continue;
            }

            glViewport(0, 0, width, height);
            glClearColor(0.08f, 0.09f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                static_cast<float>(width) /
                    static_cast<float>(height),
                0.1f,
                100.0f
            );

            glUniformMatrix4fv(
                projectionLocation, 1, GL_FALSE,
                glm::value_ptr(projection)
            );

            table.draw();
            glfwSwapBuffers(window);
        }
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        result = 1;
    }

    glDeleteProgram(program);
    glfwDestroyWindow(window);
    glfwTerminate();

    return result;
}