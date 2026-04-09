#include <GLAD/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <cmath>
#include <cstdlib>

using namespace std;

GLFWwindow* initilizeWindow();
void loadShaders();
unsigned int buildShaders();
unsigned int buildTexture();
unsigned int createBuffers();

void window_resize_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window, float deltaTime);
void readFile(string *fileSource, const char* filePath);
void simStart();
void simStep(float deltaTime);
void updateTexture();
float randFloat(float max, float min = 0.f, int precision = 100000);

// Simulation Settings
const unsigned int SIM_WIDTH = 1000;
const unsigned int SIM_HEIGHT = 800;
const unsigned int NUM_AGENTS = 50;
const float AGENT_SPEED = 1.f;

// OpenGL Settings
const unsigned int SIM_TO_SRC_MULTI = 1;
const unsigned int SCR_WIDTH = SIM_WIDTH*SIM_TO_SRC_MULTI;
const unsigned int SCR_HEIGHT = SIM_HEIGHT*SIM_TO_SRC_MULTI;

// Shaders
const char *VERT_SHADER_PATH = "src/shaders/vertShader.vert";
string VERT_SHADER;
const char *FRAG_SHADER_PATH = "src/shaders/fragShader.frag";
string FRAG_SHADER;

// Structs
struct agent {
    // X position
    float x;
    // Y position
    float y;
    // Facing direction
    float d;
};

// Simulation Variables
float points[SIM_WIDTH*SIM_HEIGHT];
struct agent* agents[NUM_AGENTS];

int main() {
    // Initilize GLFW, GLAD, and the window
    GLFWwindow* window = initilizeWindow();
    // If the window is NULL, it failed to initilize
    if (window == NULL) {
        return -1;
    }

    // Read the shaders from files
    loadShaders();
    // If either is empty, they failed to read
    if (VERT_SHADER.empty() || FRAG_SHADER.empty()) {
        return -1;
    }

    // Build and compile shader programs
    unsigned int shaderProgram = buildShaders();

    // Create the simulation texture
    unsigned int TEX = buildTexture();

    // Set up buffers
    unsigned int VAO = createBuffers();

    // Initilize the simulation and deltaTime variables
    simStart();
    chrono::steady_clock::time_point lastTime = chrono::steady_clock::now();
    float deltaTime = 0.f;
    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Update deltaTime
        chrono::steady_clock::time_point curTime = chrono::steady_clock::now();
        deltaTime = chrono::duration_cast<chrono::microseconds>(curTime - lastTime).count() / 1000000.f;

        // Input
        processInput(window, deltaTime);

        // Step simulation
        simStep(deltaTime);

        // Render
        //
        // Clear background
        glClearColor(0.5f, 0.5f, 0.5f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);
        // Draw the triangles
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
    for (int i = 0; i < NUM_AGENTS; i++) {
        delete(agents[i]);
    }
    return 0;
}

void simStart() {
    // Pass the time as the random seed
    srand(time(0));

    // Initilize the agents randomly near the center, pointing outwards
    for (int i = 0; i < NUM_AGENTS; i++) {
        // Initialize the agent
        agents[i] = new agent();

        // Get random direction
        float randomAngle = randFloat(2*acos(-1.f));
        // Get a random distance from the center
        float randomDist = randFloat(1.f);

        // DEBUGGING
        cout << randomAngle << endl;

        // Set the agents position and direction based on the random numbers
        agents[i] -> d = randomAngle;
        agents[i] -> x = SIM_WIDTH/2 + randomDist*cos(randomAngle);
        agents[i] -> y = SIM_HEIGHT/2 + randomDist*sin(randomAngle);
    }
}

void simStep(float deltaTime) {
    // For each agent, set the point it is closest to to 1
    // Then move the agent
    for (int i = 0; i < NUM_AGENTS; i++) {
        // Move the agent
        agents[i] -> x += deltaTime*AGENT_SPEED*cos(agents[i]->d);
        agents[i] -> y += deltaTime*AGENT_SPEED*sin(agents[i]->d);
        // If the agent has hit a wall, make its direction point randomly away from that wall
        // Left wall >> -pi/2 < d < pi/2
        if (agents[i] -> x < 0) {
            agents[i] -> x = 0;
            agents[i] -> d = randFloat(acos(-1.f)/2, -acos(-1.f)/2);
        }
        // Right wall >> pi/2 < d < 3pi/2
        else if (agents[i] -> x > (SIM_WIDTH-1)) {
            agents[i] -> x = SIM_WIDTH-1;
            agents[i] -> d = randFloat(3*acos(-1.f)/2, acos(-1.f)/2);
        }
        // Bottom wall >> 0 < d < pi
        else if (agents[i] -> y < 0) {
            agents[i] -> y = 0;
            agents[i] -> d = randFloat(acos(-1.f)/2);
        }
        // Top wall >> pi < d < 2pi
        else if (agents[i] -> y > (SIM_HEIGHT-1)) {
            agents[i] -> y = SIM_HEIGHT-1;
            agents[i] -> d = randFloat(2*acos(-1.f), acos(-1.f));
        }

        // Debugging
        int agentX = agents[i] -> x;
        int agentY = agents[i] -> y;

        // Get closest pixel positions
        int xPoint = static_cast<int>(agents[i] -> x);
        int yPoint = static_cast<int>(agents[i] -> y);
        // Set that point to 1
        points[yPoint*SIM_WIDTH + xPoint] = 1.f;
    }
    updateTexture();
}

/// @brief Generates a float randomly between the min and max with a step size of precision
/// @param max highest that the float can be, must be 0<M
/// @param min lowest that the float can be, must be m<M (can be m<0) [default = 0.f]
/// @param precision the step size of the random float [default = 100000]
/// @return 
float randFloat(float max, float min, int precision) {
    return min + (max - min)*(rand() % precision)/precision;
}

GLFWwindow* initilizeWindow() {
    // Initialize and configure glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, window_resize_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        glfwTerminate();
        return NULL;
    }

    return window;
}

void loadShaders() {
    // Read in shaders from the files
    //
    // Vertex shader
    readFile(&VERT_SHADER, VERT_SHADER_PATH);
    if (VERT_SHADER.empty()) {
        cout << "FILE ERROR: Could not open vertex shader file" << endl;
        glfwTerminate();
        return;
    }
    // Fragment Shader
    readFile(&FRAG_SHADER, FRAG_SHADER_PATH);
    if (FRAG_SHADER.empty()) {
        cout << "FILE ERROR: Could not open fragment shader file" << endl;
        glfwTerminate();
        return;
    }
}

unsigned int buildShaders() {
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
        cout << "SHADER ERROR: Vertex compilation failed" << infoLog << endl;
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
        cout << "SHADER ERROR: Fragment compilation failed" << infoLog << endl;
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
        cout << "SHADER ERROR: Linking failed" << infoLog << endl;
    }
    // Clean up the shaders now that linking is done
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

unsigned int buildTexture() {
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

    return TEX;
}

unsigned int createBuffers() {
    // Set up initial vertex and texcoords data
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

    return VAO;
}

/// @brief Checks whether relevent keys were pressed and performs actions accordingly
/// @param window 
void processInput(GLFWwindow *window, float deltaTime) {
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
void readFile(string *fileSource, const char* filePath) {
    // Open the file
    ifstream inputFile(filePath);
    if (!inputFile.is_open()) { *fileSource = ""; return ; }
    // Read the file
    *fileSource = string(istreambuf_iterator<char>(inputFile), istreambuf_iterator<char>());
    // Close the file
    inputFile.close();
}

void updateTexture() {
    // Update the texture object
    // Updates the entire thing at once, not super efficient but it should work for now
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SIM_WIDTH, SIM_HEIGHT, GL_RED, GL_FLOAT, points);
}