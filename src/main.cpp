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
#include <cmath>

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
std::string chairFolder =
    std::string(PROJECT_ROOT) +
    "/assets/models/painted_wooden_chair_02_1k/";

Model chair(
    chairFolder + "painted_wooden_chair_02_1k.fbx",
    chairFolder + "textures/painted_wooden_chair_02_diff_1k.jpg"
);
Model floor(
    std::string(PROJECT_ROOT) + "/assets/models/study_floor/floor.obj",
    std::string(PROJECT_ROOT) + "/assets/textures/study_floor.jpg"
);Model walls(
    std::string(PROJECT_ROOT) + "/assets/models/study_room/walls.obj",
    std::string(PROJECT_ROOT) + "/assets/textures/study_wall.jpg"
);
std::string shelfFolder =
    std::string(PROJECT_ROOT) +
    "/assets/models/Shelf_01_1k/";

Model shelf(
    shelfFolder + "Shelf_01_1k.fbx",
    shelfFolder + "textures/Shelf_01_diff_1k.jpg"
);std::string lampFolder =
    std::string(PROJECT_ROOT) +
    "/assets/models/desk_lamp_arm_01_1k/";

Model lamp(
    lampFolder + "desk_lamp_arm_01_1k.gltf",
    lampFolder + "textures/desk_lamp_arm_01_diff_1k.jpg"
);std::string clockFolder =
    std::string(PROJECT_ROOT) +
    "/assets/models/vintage_grandfather_clock_01_1k/";

Model clock(
    clockFolder + "vintage_grandfather_clock_01_1k.gltf",
    clockFolder + "textures/vintage_grandfather_clock_01_diff_1k.jpg"
);
std::string globeFolder =
    std::string(PROJECT_ROOT) +
    "/assets/models/antique_globe/";

Model globe(
    globeFolder + "Old_Antique_Standing_Globe_.fbx",
    globeFolder + "Textures/Diffuse.png"
);
        GLint modelLocation = glGetUniformLocation(program, "model");
        GLint viewLocation = glGetUniformLocation(program, "view");
        GLint projectionLocation =
            glGetUniformLocation(program, "projection");

        glUniform1i(
            glGetUniformLocation(program, "diffuseTexture"), 0
        );
        GLint directionalColorLocation =
    glGetUniformLocation(program, "directionalColor");
GLint pointColorLocation =
    glGetUniformLocation(program, "pointColor");
GLint spotColorLocation =
    glGetUniformLocation(program, "spotColor");

glUniform3f(
    glGetUniformLocation(program, "directionalDirection"),
    -0.5f, -1.0f, -0.3f
);

// Place the warm light near the lamp shade
glUniform3f(
    glGetUniformLocation(program, "pointPosition"),
    -0.65f, 0.9f, -0.1f
);

glUniform3f(
    glGetUniformLocation(program, "spotPosition"),
    1.5f, 2.5f, 2.0f
);

// Aim the spotlight from its position toward the model's centre.
glUniform3f(
    glGetUniformLocation(program, "spotDirection"),
    -1.5f, -2.5f, -2.0f
);

   glm::mat4 view = glm::lookAt(
    glm::vec3(3.0f, 1.8f, 3.5f),
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
        bool horrorMode = false;
bool hWasPressed = false;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
bool hPressed = glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS;

// Toggle once per press, rather than once per frame.
if (hPressed && !hWasPressed) {
    horrorMode = !horrorMode;

    glfwSetWindowTitle(
        window,
        horrorMode
            ? "Haunted House - Horror ON"
            : "Haunted House - Study"
    );
}

hWasPressed = hPressed;
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
            // Hold 1, 2, or 3 to isolate a light and inspect its effect
bool moonOnly = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
bool pointOnly = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
bool spotOnly = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;

bool inspecting = moonOnly || pointOnly || spotOnly;

float moonStrength = (!inspecting || moonOnly) ? 1.0f : 0.0f;
float pointStrength = (!inspecting || pointOnly) ? 1.0f : 0.0f;
float spotStrength = (!inspecting || spotOnly) ? 1.0f : 0.0f;

float time = static_cast<float>(glfwGetTime());

glm::vec3 moonColor(0.25f, 0.32f, 0.50f);
glm::vec3 pointColor(1.20f, 0.65f, 0.25f);
glm::vec3 spotColor(0.90f, 0.95f, 1.10f);

glm::vec3 spotPosition(1.5f, 2.5f, 2.0f);
glm::vec3 spotTarget(0.0f);

if (horrorMode) {
    moonColor *= 0.25f;

    // Uneven, time-based flickering.
    float flicker =
        0.55f +
        0.25f * std::sin(time * 7.0f) +
        0.15f * std::sin(time * 13.0f);

    pointColor = glm::vec3(1.0f, 0.20f, 0.08f) * flicker;
    spotColor *= 0.45f;

    spotPosition.x += 0.5f * std::sin(time * 0.7f);
    spotTarget.x = 0.7f * std::sin(time * 0.9f);
    spotTarget.z = 0.4f * std::cos(time * 0.6f);
}

moonColor *= moonStrength;
pointColor *= pointStrength;
spotColor *= spotStrength;

glm::vec3 spotDirection = spotTarget - spotPosition;

glUniform3fv(
    directionalColorLocation, 1, glm::value_ptr(moonColor)
);

glUniform3fv(
    pointColorLocation, 1, glm::value_ptr(pointColor)
);

glUniform3fv(
    spotColorLocation, 1, glm::value_ptr(spotColor)
);

glUniform3fv(
    glGetUniformLocation(program, "spotPosition"),
    1, glm::value_ptr(spotPosition)
);

glUniform3fv(
    glGetUniformLocation(program, "spotDirection"),
    1, glm::value_ptr(spotDirection)
);// Place the floor below the furniture
glm::mat4 floorModel = glm::translate(
    glm::mat4(1.0f),
    glm::vec3(0.0f, -0.65f, 0.0f)
);

floorModel = glm::scale(
    floorModel,
    glm::vec3(4.0f, 1.0f, 4.0f)
);

glUniformMatrix4fv(
    modelLocation, 1, GL_FALSE, glm::value_ptr(floorModel)
);
floor.draw();

// Match the walls to the floor edges
glm::mat4 wallModel = glm::translate(
    glm::mat4(1.0f),
    glm::vec3(0.0f, 0.85f, 0.0f)
);

wallModel = glm::scale(
    wallModel,
    glm::vec3(4.0f)
);

glUniformMatrix4fv(
    modelLocation, 1, GL_FALSE, glm::value_ptr(wallModel)
);

walls.draw();
// Keep the shelf near the back wall
glm::mat4 shelfModel = glm::translate(
    glm::mat4(1.0f),
    glm::vec3(-1.8f, 0.35f, -3.5f)
);

glUniformMatrix4fv(
    modelLocation, 1, GL_FALSE, glm::value_ptr(shelfModel)
);
shelf.draw();
// Place the clock beside the bookshelf
glm::mat4 clockModel = glm::translate(
    glm::mat4(1.0f),
    glm::vec3(-0.3f, 0.35f, -3.5f)
);

glUniformMatrix4fv(
    modelLocation, 1, GL_FALSE, glm::value_ptr(clockModel)
);
clock.draw();
// Place the globe beside the desk
glm::mat4 globeModel = glm::translate(
    glm::mat4(1.0f),
    glm::vec3(-1.8f, -0.05f, 0.3f)
);

globeModel = glm::scale(
    globeModel,
    glm::vec3(0.6f)
);

glUniformMatrix4fv(
    modelLocation, 1, GL_FALSE, glm::value_ptr(globeModel)
);
globe.draw();
glm::mat4 tableModel(1.0f);

glUniformMatrix4fv(
    modelLocation, 1, GL_FALSE, glm::value_ptr(tableModel)
);
table.draw();
// Start near the left edge of the table
glm::mat4 lampModel = glm::translate(
    glm::mat4(1.0f),
   glm::vec3(-0.7f, 0.65f, -0.4f)
);

lampModel = glm::scale(
    lampModel,
    glm::vec3(0.3f)
);

glUniformMatrix4fv(
    modelLocation, 1, GL_FALSE, glm::value_ptr(lampModel)
);
lamp.draw();
// Our loader gives every model a longest side of two units
// Scale the chair down and place it beside the table
glm::mat4 chairModel = glm::translate(
    glm::mat4(1.0f),
    glm::vec3(1.4f, 0.0f, 0.0f)
);

chairModel = glm::scale(
    chairModel,
    glm::vec3(0.65f)
);

glUniformMatrix4fv(
    modelLocation, 1, GL_FALSE, glm::value_ptr(chairModel)
);
chair.draw();
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