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

// Generate a grid of random voxel values
std::array<std::array<std::array<float, 100>, 100>, 100> generateRandomVoxelGrid()
{
    std::mt19937 engine(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::array<std::array<std::array<float, 100>, 100>, 100> ret;
    for (int i = 0; i < 100; ++i)
        for (int j = 0; j < 100; ++j)
            for (int k = 0; k < 100; ++k)
                ret[i][j][k] = dist(engine);

    return ret;
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

    // Create a shader program from files
    GLuint shaderProgram = createProgram("shaders/voxel.vert", "shaders/voxel.frag");


    // Cube Geometery
    float vertices[] = {
        // x      y      z
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
    };

    unsigned int indices[] = {
        0,1,2, 2,3,0, // back
        4,5,6, 6,7,4, // front
        0,4,7, 7,3,0, // left
        1,5,6, 6,2,1, // right
        3,2,6, 6,7,3, // top
        0,1,5, 5,4,0, // bottom
    };

    // Positions for 100x100x100 super cube
    std::vector<glm::vec3> instancePositions;
    instancePositions.reserve(100 * 100 * 100);
    for (int i = 0; i < 100; ++i)
        for (int j = 0; j < 100; ++j)
            for (int k = 0; k < 100; ++k)
                instancePositions.push_back(glm::vec3(i, j, k));


    // GPU Buffers
    GLuint VAO, VBO, EBO, instanceVBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instancePositions.size() * sizeof(glm::vec3), instancePositions.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    glVertexAttribDivisor(1, 1);

    // Shader Uniforms for MVP matrices
    GLint modelLoc      = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc       = glGetUniformLocation(shaderProgram, "view");
    GLint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

    // ( Projection computed every frame in render loop )    
    glm::mat4 view = glm::lookAt(glm::vec3(200, 150, 200), glm::vec3(50, 50, 50), glm::vec3(0, 1, 0));
    glm::mat4 model = glm::mat4(1.0f);

    // Simulation Data
    auto voxelState = generateRandomVoxelGrid();

    // Render Loop
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Upload MVP matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

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
