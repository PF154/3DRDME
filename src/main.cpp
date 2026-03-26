#include <SimData.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <array>
#include <vector>
#include <random>
#include <algorithm>
#include <future>
#include <chrono>

SimData simData;

struct Camera 
{
    float yaw = 0.0f;
    float pitch = 0.3f;
    float radius = 200.0f;

    const float cameraSpeed = 0.005f;
    const float zoomSpeed = 1.0f;
};

struct Cursor 
{
    bool moved = false;
    float x;
    float y;
};

Cursor cursor;
Camera camera;

// Reads a file into a string
std::string loadFile(const std::string& path)
{
    std::ifstream f(path);
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Compiles a single shader stage and returns its ID
GLuint compileShader(GLenum type, const std::string& src)
{
    // Create shader, set source, compile
    GLuint shader = glCreateShader(type);

    const char* srcPtr = src.c_str();
    glShaderSource(shader, 1, &srcPtr, NULL);
    glCompileShader(shader);

    // Check for errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }

    // Return shader ID
    return shader;
}

// Links vert + frag shaders into a program and returns its ID
GLuint createProgram(const std::string& vertPath, const std::string& fragPath)
{
    // Compile both stages, attach to program, link, delete shader objects
    GLuint vertShader = compileShader(GL_VERTEX_SHADER, loadFile(vertPath));
    GLuint fragShader = compileShader(GL_FRAGMENT_SHADER, loadFile(fragPath));

    GLuint program = glCreateProgram();
    glAttachShader(program, vertShader);
    glAttachShader(program, fragShader);

    glLinkProgram(program);

    // Check for errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        std::cerr << "Program link error:\n" << log << std::endl;
    }

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    // Return program ID
    return program;
}

void cursorCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (!cursor.moved)
    {
        cursor.x = xpos;
        cursor.y = ypos;
        cursor.moved = true;
    }
    else
    {
        float deltaX = xpos - cursor.x;
        float deltaY = ypos - cursor.y;

        cursor.x = xpos;
        cursor.y = ypos;

        // Move camera
        camera.yaw = std::fmod(6.283 + camera.yaw + deltaX * camera.cameraSpeed, 6.283f);
        camera.pitch = std::clamp(camera.pitch + deltaY * camera.cameraSpeed, -1.5f, 1.5f);
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.radius -= yoffset * camera.zoomSpeed;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

int main()
{
    // Initialize glfw
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "RW Sim", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetCursorPosCallback(window, cursorCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetKeyCallback(window, keyCallback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // Use modern OpenGL features
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        exit(EXIT_FAILURE);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create a shader program from files
    GLuint shaderProgram = createProgram("shaders/voxel.vert", "shaders/voxel.frag");


    // Cube Geometry — 24 vertices (4 per face), format: x, y, z, nx, ny, nz
    float vertices[] = {
        // back face (normal: 0, 0, -1)
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        // front face (normal: 0, 0, 1)
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
        // left face (normal: -1, 0, 0)
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        // right face (normal: 1, 0, 0)
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
        // top face (normal: 0, 1, 0)
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        // bottom face (normal: 0, -1, 0)
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    };

    unsigned int indices[] = {
         0, 1, 2,  2, 3, 0, // back
         4, 5, 6,  6, 7, 4, // front
         8, 9,10, 10,11, 8, // left
        12,13,14, 14,15,12, // right
        16,17,18, 18,19,16, // top
        20,21,22, 22,23,20, // bottom
    };

    // Positions for 100x100x100 super cube
    std::vector<glm::vec3> instancePositions;
    instancePositions.reserve(100 * 100 * 100);
    for (int i = 0; i < 100; ++i)
        for (int j = 0; j < 100; ++j)
            for (int k = 0; k < 100; ++k)
                instancePositions.push_back(glm::vec3(i, j, k));


    // GPU Buffers
    GLuint VAO, VBO, EBO, instanceVBO, concentrationVBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instancePositions.size() * sizeof(glm::vec3), instancePositions.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1);

    glGenBuffers(1, &concentrationVBO);
    glBindBuffer(GL_ARRAY_BUFFER, concentrationVBO);
    glBufferData(GL_ARRAY_BUFFER, 100*100*100 * sizeof(float), simData.getSimData(), GL_STATIC_DRAW);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);

    // Shader Uniforms for MVP matrices
    GLint modelLoc      = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc       = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint lightPosLoc   = glGetUniformLocation(shaderProgram, "lightPos");
    GLint viewPosLoc    = glGetUniformLocation(shaderProgram, "viewPos");

    // ( Projection computed every frame in render loop )    
    glm::mat4 model = glm::mat4(1.0f);

    // Render Loop
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Compute camera position and view matrix
        float camX = 50 + camera.radius * cos(camera.pitch) * cos(camera.yaw);
        float camY = 50 + camera.radius * sin(camera.pitch);
        float camZ = 50 + camera.radius * cos(camera.pitch) * sin(camera.yaw);
        glm::mat4 view = glm::lookAt(glm::vec3(camX, camY, camZ), glm::vec3(50, 50, 50), glm::vec3(0, 1, 0));


        // Upload MVP matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(lightPosLoc, 1, glm::value_ptr(glm::vec3(200.0f, 200.0f, 200.0f)));
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(glm::vec3(camX, camY, camZ)));

        if (simData.update())
        {
            float* data = simData.getSimData();
            glBindBuffer(GL_ARRAY_BUFFER, concentrationVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 100*100*100 * sizeof(float), data);
        }

        // Draw
        glBindVertexArray(VAO);
        glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, 100*100*100);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
