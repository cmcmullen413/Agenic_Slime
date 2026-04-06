#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>

// TEMP
#include <filesystem>

void window_resize_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void readFile(std::string *fileSource, const char* filePath);

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Shaders
const char *VERT_SHADER_PATH = "src/shaders/vertShader.vert";
std::string VERT_SHADER;
const char *FRAG_SHADER_PATH = "src/shaders/fragShader.frag";
std::string FRAG_SHADER;

int main() {
    // Initialize and configure glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, window_resize_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Read in shaders from the files
    //
    // Vertex shader
    readFile(&VERT_SHADER, VERT_SHADER_PATH);
    if (VERT_SHADER.empty()) {
        std::cout << "FILE ERROR: Could not open vertex shader file" << std::endl;
        glfwTerminate();
        return -1;
    }
    // Fragment Shader
    readFile(&FRAG_SHADER, FRAG_SHADER_PATH);
    if (FRAG_SHADER.empty()) {
        std::cout << "FILE ERROR: Could not open fragment shader file" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Build and compile shader programs
    //
    // Vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char *vertSource = VERT_SHADER.c_str();
    glShaderSource(vertexShader, 1, &vertSource, NULL);
    glCompileShader(vertexShader);
    // Check for errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "SHADER ERROR: Vertex compilation failed" << infoLog << std::endl;
    }
    // Fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fragSource = FRAG_SHADER.c_str();
    glShaderSource(fragmentShader, 1, &fragSource, NULL);
    glCompileShader(fragmentShader);
    // check for errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "SHADER ERROR: Fragment compilation failed" << infoLog << std::endl;
    }
    // Link the shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // Check for errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "SHADER ERROR: Linking failed" << infoLog << std::endl;
    }
    // Clean up the shaders now that linking is done
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up initial vertex data and color data
    float bufferData[] = {
        // Two quads, one for top left, one for bottom right

        // Top left quad (Black)
        -1.f, 1.f, 0.f, 0.2f, 0.2f, 0.2f,
        -1.f, 0.f, 0.f, 0.2f, 0.2f, 0.2f,
        0.f, 1.f, 0.f, 0.2f, 0.2f, 0.2f,

        -1.f, 0.f, 0.f, 0.2f, 0.2f, 0.2f,
        0.f, 0.f, 0.f, 0.2f, 0.2f, 0.2f,
        0.f, 1.f, 0.f, 0.2f, 0.2f, 0.2f,


        // Bottom right quad (White)
        0.f, 0.f, 0.f, 1.f, 1.f, 1.f,
        0.f, -1.f, 0.f, 1.f, 1.f, 1.f,
        1.f, 0.f, 0.f, 1.f, 1.f, 1.f,

        0.f, -1.f, 0.f, 1.f, 1.f, 1.f,
        1.f, -1.f, 0.f, 1.f, 1.f, 1.f,
        1.f, 0.f, 0.f, 1.f, 1.f, 1.f
    };

    // Set up buffers
    //
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind the Vertex Array Obeject
    glBindVertexArray(VAO);
    // Bind and set Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(bufferData), bufferData, GL_STATIC_DRAW);
    // Configure VBO attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (GLvoid*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (GLvoid*)(3*sizeof(GL_FLOAT)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    // Unbind the VAO so theres no risk of accidental modification later
    glBindVertexArray(0);

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Render
        glClearColor(0.f, 0.f, 1.f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the quads
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 12);
        glBindVertexArray(0);

        // Swap buffers and poll IO
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate glfw to clear resources
    glfwTerminate();
    return 0;
}

/// @brief Checks whether relevent keys were pressed and performs actions accordingly
/// @param window 
void processInput(GLFWwindow *window) {
    // Close the window if ESC is pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

/// @brief Updates the windows size whenever something triggers a resize
/// @param window 
/// @param width 
/// @param height 
void window_resize_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

/// @brief Reads all the text from a file
/// @param fileSource the string to save the data too
/// @param filePath 
void readFile(std::string *fileSource, const char* filePath) {
    // Open the file
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) { *fileSource = ""; return ; }
    // Read the file
    *fileSource = std::string(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());
    // Close the file
    inputFile.close();
}