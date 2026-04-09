#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>

void window_resize_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void readFile(std::string *fileSource, const char* filePath);
void simStart();
void simStep();
void updateTexture();

// Settings
const unsigned int SIM_WIDTH = 100;
const unsigned int SIM_HEIGHT = 100;
const unsigned int SIM_TO_SRC_MULTI = 8;
const unsigned int SCR_WIDTH = SIM_WIDTH*SIM_TO_SRC_MULTI;
const unsigned int SCR_HEIGHT = SIM_HEIGHT*SIM_TO_SRC_MULTI;

// Shaders
const char *VERT_SHADER_PATH = "src/shaders/vertShader.vert";
std::string VERT_SHADER;
const char *FRAG_SHADER_PATH = "src/shaders/fragShader.frag";
std::string FRAG_SHADER;

// Simulation Variables
float points[SIM_WIDTH*SIM_HEIGHT];

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

    // Create the simulation texture
    unsigned int TEX;
    glGenTextures(1, &TEX);
    glBindTexture(GL_TEXTURE_2D, TEX);
    // Make the texture display as grey outside of its area
    float color[] = {0.3f, 0.3f, 0.3f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);
    // Tell the texture what to do if the screen pixel doesn't match up with the texture pixel
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // Fill texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SIM_WIDTH, SIM_HEIGHT, 0, GL_RED, GL_FLOAT, points);
    simStart();


    // Set up initial vertex, color, and texcoords data
    float vertices[] = {
    //  Position        Tex Coords
        -1.f, 1.f,      0.f, 1.f,
        1.f, 1.f,       1.f, 1.f,
        1.f, -1.f,      1.f, 0.f,

        -1.f, 1.f,      0.f, 1.f,
        1.f, -1.f,      1.f, 0.f,
        -1.f, -1.f,     0.f, 0.f
    };
    float strideLength = 4*sizeof(float);

    // Set up buffers
    //
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // Bind the Vertex Array Obeject
    glBindVertexArray(VAO);
    // Bind and set Vertex Buffer Object
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Configure VBO attributes
    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, strideLength, (GLvoid*)0);
    // Tex coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, strideLength, (GLvoid*)(2*sizeof(GL_FLOAT)));
    // Enables the attributes
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    // Unbind the VAO and VBO so theres no risk of accidental modification later
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Input
        processInput(window);

        // Render
        //
        // Clear background
        glClearColor(0.5f, 0.5f, 0.5f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Draw the quads
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Swap buffers and poll IO
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate glfw to clear resources
    glfwTerminate();
    return 0;
}

// Key press state holders
bool SPACE_PRESSED;
bool R_PRESSED;

/// @brief Checks whether relevent keys were pressed and performs actions accordingly
/// @param window 
void processInput(GLFWwindow *window) {
    // Close the window if ESC is pressed
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    // Call a simulation step if SPACE is pressed
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!SPACE_PRESSED) { simStep(); }
        SPACE_PRESSED = true;
    } else { SPACE_PRESSED = false; }
    // Reset the simulaton if R is pressed
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        if (!R_PRESSED) { simStart(); }
        R_PRESSED = true;
    } else { R_PRESSED = false; }
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

void simStart() {
    std::cout << "Sim Start" << std::endl;
    for (int x = 0; x < SIM_WIDTH; x++) {
        for (int y = 0; y < SIM_HEIGHT; y++) {
            int pos = SIM_WIDTH*y + x;
            // If x is even, turn on the pixel
            if (x % 2 == 0) {
                points[pos] = 1.f;
            }
            else {
                points[pos] = 0.f;
            }
        }
    }
    updateTexture();
}

void simStep() {
    std::cout << "Sim Step" << std::endl;
    for (int i = 0; i < 3*SIM_WIDTH*SIM_HEIGHT; i++) {
        // Increment the color everystep
        points[i] += 0.1f;
        // Keeps the colors between 0 and 1
        if (points[i] > 1.f) {
            points[i] -= 1.f;
        }
    }
    updateTexture();
}

void updateTexture() {
    // Update the texture object
    // Updates the entire thing at once, not super efficient but it should work for now
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SIM_WIDTH, SIM_HEIGHT, GL_RED, GL_FLOAT, points);
}