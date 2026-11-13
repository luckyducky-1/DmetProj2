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

    GLuint program;
    try {
        program = createShaderProgram();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // Eight corners, shared by the cube's twelve triangles.
    const float vertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };

    const unsigned int indices[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 4, 7, 7, 3, 0,
        1, 5, 6, 6, 2, 1,
        3, 2, 6, 6, 7, 3,
        0, 1, 5, 5, 4, 0
    };

    GLuint vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(
        GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr
    );
    glEnableVertexAttribArray(0);

    glUseProgram(program);

    GLint modelLocation = glGetUniformLocation(program, "model");
    GLint viewLocation = glGetUniformLocation(program, "view");
    GLint projectionLocation = glGetUniformLocation(program, "projection");
    GLint colorLocation = glGetUniformLocation(program, "objectColor");

    glm::mat4 view = glm::lookAt(
        glm::vec3(2.5f, 1.8f, 3.5f),
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));

    const glm::vec3 faceColors[] = {
        {0.55f, 0.28f, 0.18f},
        {0.85f, 0.55f, 0.30f},
        {0.65f, 0.38f, 0.22f},
        {0.75f, 0.45f, 0.25f},
        {0.95f, 0.70f, 0.40f},
        {0.40f, 0.22f, 0.15f}
    };

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
            static_cast<float>(width) / static_cast<float>(height),
            0.1f,
            100.0f
        );

        // Elapsed time keeps rotation independent of frame rate.
        float angle = static_cast<float>(glfwGetTime()) * 0.5f;
        glm::mat4 model = glm::rotate(
            glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glUniformMatrix4fv(
            modelLocation, 1, GL_FALSE, glm::value_ptr(model)
        );
        glUniformMatrix4fv(
            projectionLocation, 1, GL_FALSE, glm::value_ptr(projection)
        );

        glBindVertexArray(vao);

        // Different face colours make the cube's shape easy to see.
        for (int face = 0; face < 6; ++face) {
            glUniform3fv(colorLocation, 1, glm::value_ptr(faceColors[face]));

            auto offset = static_cast<std::size_t>(face) *
                          6 * sizeof(unsigned int);

            glDrawElements(
                GL_TRIANGLES, 6, GL_UNSIGNED_INT,
                reinterpret_cast<const void*>(offset)
            );
        }

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}