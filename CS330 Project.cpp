#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>  
#define STB_IMAGE_IMPLEMENTATION // GLFW library
#include "Debug/stb_image.h"
// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Debug/camera.h"

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "CS330 Project"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao, vao1, vao2, vao3, vao4, vao5, vao6, vao7, vao8, vao9, vao10, vao11, vao12, vao13, vao14;         // Handle for the vertex array object
        GLuint vbos[2], vbos1[2], vbos2[2], vbos3[2], vbos4[2], vbos5[2], vbos6[2], vbos7[2], vbos8[2], vbos9[2], vbos10[2], vbos11[2], vbos12[2], vbos13[2], vbos14[2];     // Handles for the vertex buffer objects
        GLuint nIndices, nRoofIndices, nGrassIndices, nDriveWayIndices, nSecondBaseIndices, nTopHouseIndices, nRightHouseIndices, nLeftHouseIndices,
            nTopRoofIndices, nWindowIndices, nWalkUpIndices, nFrontDoorIndices, nGarageIndices, nFrontWindowIndices, nFenceIndices, nLampIndices;    // Number of indices of the mesh
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture
    GLuint gTextureId, roofTextureId, grassTextureId, drivewayTextureId, tophouseTextureId, rightleftHouseTextureID, topWindowTextureId, 
        frontDoorTextureId, garageTextureId, frontWindowTextureId, fenceTextureId;
    glm::vec2 gUVScale(1.0f, 1.0f);
    glm::vec2 gRoofScale(2.0f, 2.0f);
    glm::vec2 gGrassScale(1.0f, 1.0f);
    glm::vec2 gDrivewayScale(1.0f, 1.0f);
    GLint gTextWrapMode = GL_REPEAT;

    // Shader program
    GLuint gProgramId;
    GLuint gLampProgramId;

    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0F;
    bool gFirstMouse = true;
    float gDeltaTime = 0.0f;
    float gLastFrame = 0.0f;

    // Object and light color
    glm::vec3 gObjectColor(1.0f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);

    // Light position and scale
    glm::vec3 gLightPosition(-8.0f, 6.0f, 6.0f);
    glm::vec3 gLightScale(0.3f);

    bool gIsLampOrbiting = true;
}

/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);


/* Vertex Shader Source Code*/
const GLchar* objectVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
layout(location = 1) in vec3 normal; // VAP position 1 for normals
layout(location = 2) in vec2 textureCoordinate;

out vec3 vertexNormal; // For outgoing normals to fragment shader
out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
out vec2 vertexTextureCoordinate;

//Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

    vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

    vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
    vertexTextureCoordinate = textureCoordinate;
}
);


/* Fragment Shader Source Code*/
const GLchar* objectFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
in vec3 vertexFragmentPos; // For incoming fragment position
in vec2 vertexTextureCoordinate;

out vec4 fragmentColor; // For outgoing cube color to the GPU

// Uniform / Global variables for object color, light color, light position, and camera/view position
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPosition;
uniform sampler2D uTexture; // Useful when working with multiple textures
uniform vec2 uvScale;

void main()
{
    /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

    //Calculate Ambient lighting*/
    float ambientStrength = 0.3f; // Set ambient or global lighting strength
    vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

    //Calculate Diffuse lighting*/
    vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
    vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
    float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
    vec3 diffuse = impact * lightColor; // Generate diffuse light color

    //Calculate Specular lighting*/
    float specularIntensity = 0.8f; // Set specular light strength
    float highlightSize = 16.0f; // Set specular highlight size
    vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
    vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
    //Calculate specular component
    float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
    vec3 specular = specularIntensity * specularComponent * lightColor;

    // Texture holds the color to be used for all three components
    vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

    // Calculate phong result
    vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

    fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
}
);

/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

        //Uniform / Global variables for the  transform matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
}
);

/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
{
    fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
}
);

// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader program
    if (!UCreateShaderProgram(objectVertexShaderSource, objectFragmentShaderSource, gProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Load texture
    const char* texFilename = "House Texture.jpg";
    if (!UCreateTexture(texFilename, gTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    
    texFilename = "Roof Tile.jpg";
    if (!UCreateTexture(texFilename, roofTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "Kentucky Bluegrass Lawn.jpg";
    if (!UCreateTexture(texFilename, grassTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "Driveway.jpg";
    if (!UCreateTexture(texFilename, drivewayTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "Side House Texture.jpg";
    if (!UCreateTexture(texFilename, tophouseTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "RightLeftHouseTexture.jpg";
    if (!UCreateTexture(texFilename, rightleftHouseTextureID))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "TopHouseWindowTexture.jpg";
    if (!UCreateTexture(texFilename, topWindowTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "FrontDoor.jpg";
    if (!UCreateTexture(texFilename, frontDoorTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "Garage.jpg";
    if (!UCreateTexture(texFilename, garageTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "OfficeWindow.jpg";
    if (!UCreateTexture(texFilename, frontDoorTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }

    texFilename = "Fence.jpg";
    if (!UCreateTexture(texFilename, fenceTextureId))
    {
        cout << "Failed to load texture " << texFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    

    /*glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 1);

    

    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 2);*/

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.74902f, 0.847059f, 0.847059f, 1.0f);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release texture
    UDestroyTexture(gTextureId);
    UDestroyTexture(roofTextureId);
    UDestroyTexture(grassTextureId);
    UDestroyTexture(drivewayTextureId);
    UDestroyTexture(tophouseTextureId);
    UDestroyTexture(rightleftHouseTextureID);
    UDestroyTexture(topWindowTextureId);
    UDestroyTexture(frontDoorTextureId);
    UDestroyTexture(garageTextureId);
    UDestroyTexture(frontWindowTextureId);
    UDestroyTexture(fenceTextureId);

    // Release shader program
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(RIGHT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(UP, gDeltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        gCamera.ProcessKeyboard(DOWN, gDeltaTime);
    }
    
    // Pause and resume lamp orbiting
    static bool isLKeyDown = false;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !gIsLampOrbiting)
        gIsLampOrbiting = true;
    else if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS && gIsLampOrbiting)
        gIsLampOrbiting = false;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (gFirstMouse) {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos;

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}

void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}


// Functioned called to render a frame
void URender()
{

    const float angularVelocity = glm::radians(45.0f);
    if (gIsLampOrbiting)
    {
        glm::vec4 newPosition = glm::rotate(angularVelocity * gDeltaTime, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(gLightPosition, 1.0f);
        gLightPosition.x = newPosition.x;
        gLightPosition.y = newPosition.y;
        gLightPosition.z = newPosition.z;
    }

    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.196078f, 0.6f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the VBOs contained within the mesh's VAO
    glBindVertexArray(gMesh.vao);

    // Set the shader to be used
    glUseProgram(gProgramId);

    // 1. Scales the object by 2
    glm::mat4 scale = glm::scale(glm::vec3(2.0f, 2.0f, 2.0f));
    // 2. Rotates shape by 15 degrees in the x axis
    glm::mat4 rotation = glm::rotate(50.0f, glm::vec3(0.0, 1.0f, 0.0f));
    // 3. Place object at the origin
    glm::mat4 translation = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));
    // Model matrix: transformations are applied right-to-left order
    //glm::mat4 model = translation * rotation * scale;
    
    glm::mat4 model =  rotation * translation * scale;

    // Transforms the camera: move the camera back (z axis)
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a orthographic projection
    //glm::mat4 projection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 100.0f);

    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gProgramId, "view");
    GLint projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL); // Draws the triangle

    

//===========================================================================================================================================================
    // Roof Render
    glBindVertexArray(gMesh.vao1);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gRoofScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, roofTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nRoofIndices, GL_UNSIGNED_SHORT, NULL);

//===========================================================================================================================================
    // Grass Render
    glBindVertexArray(gMesh.vao2);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gGrassScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, grassTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nGrassIndices, GL_UNSIGNED_SHORT, NULL);

//==================================================================================================================================================================
//     Driveway Render
    
    glBindVertexArray(gMesh.vao3);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gDrivewayScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, drivewayTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nDriveWayIndices, GL_UNSIGNED_SHORT, NULL);

//==================================================================================================================================================================
    // Second Base

    glBindVertexArray(gMesh.vao4);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nSecondBaseIndices, GL_UNSIGNED_SHORT, NULL);

//===========================================================================================================================================================
    // Top House

    glBindVertexArray(gMesh.vao5);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tophouseTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nTopHouseIndices, GL_UNSIGNED_SHORT, NULL);

//============================================================================================================================================================
    //Right House

    glBindVertexArray(gMesh.vao6);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rightleftHouseTextureID);
    glDrawElements(GL_TRIANGLES, gMesh.nRightHouseIndices, GL_UNSIGNED_SHORT, NULL);

//===========================================================================================================================================
    //Left House

    glBindVertexArray(gMesh.vao7);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rightleftHouseTextureID);
    glDrawElements(GL_TRIANGLES, gMesh.nLeftHouseIndices, GL_UNSIGNED_SHORT, NULL);

//===========================================================================================================================================
    // Top Roof

    glBindVertexArray(gMesh.vao8);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gRoofScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, roofTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nTopRoofIndices, GL_UNSIGNED_SHORT, NULL);

//=======================================================================================================================
    // Top Windows

    glBindVertexArray(gMesh.vao9);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, topWindowTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nWindowIndices, GL_UNSIGNED_SHORT, NULL);

//==========================================================================================================================
    // Front Step

    glBindVertexArray(gMesh.vao10);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, drivewayTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nWalkUpIndices, GL_UNSIGNED_SHORT, NULL);

//=========================================================================================================================================================
    //Front Door

    glBindVertexArray(gMesh.vao11);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frontDoorTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nFrontDoorIndices, GL_UNSIGNED_SHORT, NULL);

//=============================================================================================================================================================
    // Garage

    glBindVertexArray(gMesh.vao12);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, garageTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nGarageIndices, GL_UNSIGNED_SHORT, NULL);

//=======================================================================================================================
    // Office Windows

    glBindVertexArray(gMesh.vao13);
    // Retrieves and passes transform matrices to the Shader program
    modelLoc = glGetUniformLocation(gProgramId, "model");
    viewLoc = glGetUniformLocation(gProgramId, "view");
    projLoc = glGetUniformLocation(gProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    // Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    //const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, frontWindowTextureId);
    glDrawElements(GL_TRIANGLES, gMesh.nFrontWindowIndices, GL_UNSIGNED_SHORT, NULL);

//===============================================================================================================
    // Fence

    //glBindVertexArray(gMesh.vao14);
    //// Retrieves and passes transform matrices to the Shader program
    //modelLoc = glGetUniformLocation(gProgramId, "model");
    //viewLoc = glGetUniformLocation(gProgramId, "view");
    //projLoc = glGetUniformLocation(gProgramId, "projection");

    //glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    //glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //// Reference matrix uniforms from the Object Shader program for the cub color, light color, light position, and camera position
    //objectColorLoc = glGetUniformLocation(gProgramId, "objectColor");
    //lightColorLoc = glGetUniformLocation(gProgramId, "lightColor");
    //lightPositionLoc = glGetUniformLocation(gProgramId, "lightPos");
    //viewPositionLoc = glGetUniformLocation(gProgramId, "viewPosition");

    //// Pass color, light, and camera data to the Object Shader program's corresponding uniforms
    //glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    //glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    //glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    ////const glm::vec3 cameraPosition = gCamera.Position;
    //glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    //UVScaleLoc = glGetUniformLocation(gProgramId, "uvScale");
    //glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, fenceTextureId);
    //glDrawElements(GL_TRIANGLES, gMesh.nFenceIndices, GL_UNSIGNED_SHORT, NULL);

    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draws the triangles
    glDrawElements(GL_TRIANGLES, gMesh.nIndices, GL_UNSIGNED_SHORT, NULL);
    
    // Deactivate the Vertex Array Object
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Position and Color data
    GLfloat verts[] = {

        //Square for Base (Garage Section)
        // Vertex Positions    // Normals
        //----------------------------------------------------------------------------------------
        //Front Face          //Positive Z Normal   // Texture Coords
         0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f, // Top Right (Front) - Vertex 0
         0.5f, -0.2f, 0.0f,   0.0f, 0.0f, 1.0f,    1.0f, 0.0f, // Bottom Right (Front) - Vertex 1
        -0.5f, -0.2f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f, // Bottom Left (Front) - Vertex 2
        -0.5f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, // Top Left (Front) - Vertex 3

         //Back Face          // Negative Z Normal  // Texture Coords
         0.5f, -0.2f, -0.6f,  0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Bottom Right (Back) - Vertex 4
         0.5f,  0.5f, -0.6f,  0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Top Right (Back) - Vertex 5
        -0.5f,  0.5f, -0.6f,  0.0f, 0.0f, -1.0f,   1.0f, 0.0f, // Top Left (Back) - Vertex 6
        -0.5f, -0.2f, -0.6f,  0.0f, 0.0f, -1.0f,   0.0f, 0.0f // Bottom Left (Back) - Vertex 7
    };

    // Index data to share position data
    GLushort indices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3,   // Triangle 2
        0, 1, 4,  // Triangle 3
        0, 4, 5,  // Triangle 4
        0, 5, 6, // Triangle 5
        0, 3, 6,  // Triangle 6
        4, 5, 6, // Triangle 7
        4, 6, 7, // Triangle 8
        2, 3, 6, // Triangle 9
        2, 6, 7, // Triangle 10
        1, 4, 7, // Triangle 11
        1, 2, 7 // Triangle 12
    };

    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    // Strides between vertex coordinates is 6 (x, y, z, r, g, b, a). A tightly packed stride is 0.
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerNormal + floatsPerUV);// The number of floats before each

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nIndices = sizeof(indices) / sizeof(indices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //=====================================================================================================================================================

    GLfloat roof[] = {
        //Pyramid for Base Roof (First Level Roof)
         0.0f,  1.0f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f, // Top Center Point (Right) - Vertex 8 / 0
        -1.49f,  1.0f, -0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f, // Top Center Point (Left) - Vertex 9 / 1
         0.6f, 0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    1.0f, 1.0f, // Bottom Right (Front) - Vertex 10 / 2
         0.6f, 0.5f, -0.7f,    0.0f, 0.0f, -1.0f,   1.0f, 1.0f, // Bottom Right (Back) - Vertex 11 / 3
        -1.49f, 0.5f, 0.1f,     0.0f, 0.0f, 1.0f,    0.0f, 1.0f, // Bottom Left (Front) - Vertex 12 / 4
        -1.49f, 0.5f, -0.7f,    0.0f, 0.0f, -1.0f,   0.0f, 0.0f // Bottom Left (Back) - Vertex 13 / 5
    };

    GLushort roofIndices[] = {
        // Roof Triangles
        //8, 10, 11, // Triangle 1   0, 2, 3
        //11, 8, 10, // Triangle 2   3, 1, 2
        //8, 12, 10, // Triangle 3   0, 4, 2
        //12, 9, 13, // Triangle 4   4, 1, 5
        //9, 13, 12, // Triangle 5   1, 5, 4
        //9, 8, 12, // Trianlge 6    1, 0, 4
        //9, 8, 13, // Triangle 7    1, 0, 5
        //13, 11, 8, // Triangle 8   5, 3, 0
        //11, 12, 13, // Triangle 9  3, 4, 5
        //11, 10, 12,// Triangle 10  3, 2, 4
        0, 2, 3,
        3, 0, 2,
        0, 4, 2,
        4, 1, 5,
        1, 5, 4,
        1, 0, 4,
        1, 0, 5,
        5, 3, 0,
        3, 4, 5,
        3, 2, 4
    };


    // Roof VAO and VBO binding

    glGenVertexArrays(1, &mesh.vao1); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao1);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos1);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos1[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(roof), roof, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nRoofIndices = sizeof(roofIndices) / sizeof(roofIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos1[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(roofIndices), roofIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //=================================================================================================================================================================
        // Grass 
    GLfloat grass[] = {
        //Plane for Ground
        -2.0f, -0.2f, 2.0f,   0.0f, 1.0f, 0.0f,      0.0f, 0.0f, //Front (Left) - Vertex 14 / 0
         1.5f, -0.2f, 2.0f,   0.0f, 1.0f, 0.0f,      1.0f, 0.0f, //Front (Right) - Vertex 15 / 1
         1.5f, -0.2f, -2.0f,  0.0f, 1.0f, 0.0f,      1.0f, 1.0f, //Back (Right) - Vertex 16 / 2
        -2.0f, -0.2f, -2.0f,  0.0f, 1.0f, 0.0f,      0.0f, 1.0f  //Back (Left) - Vertex 17 / 3
    };

    GLushort grassIndices[] = {
        //Ground Plane
        //14, 15, 16, // Triangle 1   1, 2, 3
        //16, 14, 17, // Triangle 2   3, 1, 4
        //17, 16, 14,  // Triangle 3   4, 3, 1
        0, 1, 2,
        2, 0, 3,
        3, 2, 0


    };

    glGenVertexArrays(1, &mesh.vao2); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao2);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos2);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos2[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(grass), grass, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nGrassIndices = sizeof(grassIndices) / sizeof(grassIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos2[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(grassIndices), grassIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //=============================================================================================================================================
        //Driveway

    GLfloat driveway[] = {
        0.3f, -0.19f, 0.0f,        0.0f, 1.0f, 0.0f,       1.0f, 1.0f, // Back Right - 0
       -0.3f, -0.19f, 0.0f,        0.0f, 1.0f, 0.0f,       0.0f, 1.0f, // Back Left - 1
        0.3f, -0.19f, 2.0f,        0.0f, 1.0f, 0.0f,       1.0f, 0.0f, // Front Right - 2
       -0.3f, -0.19f, 2.0f,        0.0f, 1.0f, 0.0f,       0.0f, 0.0f // Front Left - 3
    };

    GLushort drivewayIndices[] = {
        0, 1, 3,
        2, 0, 3,
        3, 2, 0


    };

    glGenVertexArrays(1, &mesh.vao3); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao3);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos3);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos3[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(driveway), driveway, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nDriveWayIndices = sizeof(drivewayIndices) / sizeof(drivewayIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos3[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(drivewayIndices), drivewayIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //================================================================================================================================================
        //Second Base

    GLfloat secondBase[] = {
        // Front Face

        -1.49f, 0.5f, -0.2f,     0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left 
        -1.49f, -0.2f, -0.2f,    0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        -0.5f, -0.2f, -0.2f,    0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        -0.5f, 0.5f, -0.2f,     0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right

        // Back Face

        -1.49f, 0.5f, -0.6f,     0.0f, 0.0f, -1.0f,      1.0f, 0.0f, // Top Left
        -1.49f, -0.2f, -0.6f,    0.0f, 0.0f, -1.0f,      0.0f, 0.0f, // Bottom Left
        -0.5f, -0.2f, -0.6f,    0.0f, 0.0f, -1.0f,      1.0f, 0.0f, // Bottom Right
        -0.5f, 0.5f, -0.6f,     0.0f, 0.0f, -1.0f,      1.0f, 1.0f  // Top Right
    };

    GLushort secondBaseIndices[] = {
        0, 1, 3,  // Triangle 1
        1, 2, 3,   // Triangle 2
        0, 1, 4,  // Triangle 3
        0, 4, 5,  // Triangle 4
        0, 5, 6, // Triangle 5
        0, 3, 6,  // Triangle 6
        4, 5, 6, // Triangle 7
        4, 6, 7, // Triangle 8
        2, 3, 6, // Triangle 9
        2, 6, 7, // Triangle 10
        1, 4, 7, // Triangle 11
        1, 2, 7, // Triangle 12
        4, 5, 1
    };

    glGenVertexArrays(1, &mesh.vao4); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao4);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos4);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos4[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondBase), secondBase, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nSecondBaseIndices = sizeof(secondBaseIndices) / sizeof(secondBaseIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos4[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(secondBaseIndices), secondBaseIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //===============================================================================================================================================
        // Top House

    GLfloat topHouse[] = {
        //Front Face
        -1.5f, 1.3f, -0.21f,        0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left
        -1.5f, -0.2f, -0.21f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        0.0f, -0.2f, -0.21f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        0.0f, 1.3f, -0.21f,         0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right

        //Back Face
        -1.5f, 1.3f, -1.0f,         0.0f, 0.0f, -1.0f,      1.0f, 0.0f, // Top Left
        -1.5f, -0.2f, -1.0f,        0.0f, 0.0f, -1.0f,      0.0f, 0.0f, // Bottom Left
        0.0f, -0.2f, -1.0f,         0.0f, 0.0f, -1.0f,      1.0f, 0.0f, // Bottom Right
        0.0f, 1.3f, -1.0f,         0.0f, 0.0f, -1.0f,      1.0f, 1.0f  // Top Right
    };

    GLushort tophouseIndices[] = {
        0, 1, 2,
        2, 0, 3,
        3, 2, 6,
        6, 3, 7,
        7, 6, 5,
        5, 7, 4,
        4, 5, 1,
        1, 0, 5,
        5, 4, 0,
        0, 7, 4,
        3, 7, 0

    };

    glGenVertexArrays(1, &mesh.vao5); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao5);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos5);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos5[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(topHouse), topHouse, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nTopHouseIndices = sizeof(tophouseIndices) / sizeof(tophouseIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos5[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(tophouseIndices), tophouseIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //===========================================================================================================================================
        // Right House (Moms Room)
    GLfloat rightHouse[] = {
        -0.01f, 1.3f, -0.15f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        -0.01f, 0.7f, -0.15f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        -0.51f, 0.7f, -0.15f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        -0.51f, 1.3f, -0.15f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left
        -0.26f, 1.5f, -0.15f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Point

        // Back
        -0.01f, 1.3f, -0.5f,         0.0f, 0.0f, -1.0f,       1.0f, 1.0f, // Top Right
        -0.01f, 0.7f, -0.5f,         0.0f, 0.0f, -1.0f,       1.0f, 0.0f, // Bottom Right
        -0.51f, 0.7f, -0.5f,        0.0f, 0.0f, -1.0f,       0.0f, 0.0f, // Bottom Left
        -0.51f, 1.3f, -0.5f,        0.0f, 0.0f, -1.0f,       0.0f, 1.0f, // Top Left
        -0.26f, 1.5f, -0.5f,        0.0f, 0.0f, -1.0f,       0.0f, 1.0f, // Point
    };

    GLushort righthouseIndices[] = {
        0, 1, 2,
        2, 0, 3,
        3, 2, 7,
        7, 3, 8,
        8, 7, 6,
        6, 8, 5,
        5, 6, 1,
        1, 5, 0,
        0, 4, 3,
        3, 4, 8,
        8, 9, 4,
        4, 5, 9,
        5, 4, 0,
        3, 0, 4,
        8, 9, 5,
        5, 8, 9
    };

    glGenVertexArrays(1, &mesh.vao6); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao6);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos6);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos6[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(rightHouse), rightHouse, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nRightHouseIndices = sizeof(righthouseIndices) / sizeof(righthouseIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos6[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(righthouseIndices), righthouseIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //=============================================================================================================================================
        //Left House (Dads Room)

    GLfloat leftHouse[] = {
        -1.49f, 1.3f, -0.15f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        -1.49f, 0.7f, -0.15f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        -0.99f, 0.7f, -0.15f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        -0.99f, 1.3f, -0.15f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left
        -1.24f, 1.5f, -0.15f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Point

        // Back
        -1.49f, 1.3f, -0.5f,         0.0f, 0.0f, -1.0f,       1.0f, 1.0f, // Top Right
        -1.49f, 0.7f, -0.5f,         0.0f, 0.0f, -1.0f,       1.0f, 0.0f, // Bottom Right
        -0.99f, 0.7f, -0.5f,        0.0f, 0.0f, -1.0f,       0.0f, 0.0f, // Bottom Left
        -0.99f, 1.3f, -0.5f,        0.0f, 0.0f, -1.0f,       0.0f, 1.0f, // Top Left
        -1.24f, 1.5f, -0.5f,        0.0f, 0.0f, -1.0f,       0.0f, 1.0f, // Point
    };

    GLushort lefthouseIndices[] = {
        0, 1, 2,
        2, 0, 3,
        3, 2, 7,
        7, 3, 8,
        8, 7, 6,
        6, 8, 5,
        5, 6, 1,
        1, 5, 0,
        0, 4, 3,
        3, 4, 8,
        8, 9, 4,
        4, 5, 9,
        5, 4, 0,
        3, 0, 4,
        8, 9, 5,
        5, 8, 9
    };

    glGenVertexArrays(1, &mesh.vao7); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao7);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos7);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos7[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(leftHouse), leftHouse, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nLeftHouseIndices = sizeof(lefthouseIndices) / sizeof(lefthouseIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos7[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(lefthouseIndices), lefthouseIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //=========================================================================================================================================
        // Top Roof

    GLfloat topRoof[] = {

        // Right
        0.01f, 1.31f, -0.13f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        0.01f, 1.31f, -1.01f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Back Right
        -0.26f, 1.51f, -0.15f,       0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Point
        -0.53f, 1.31f, -0.13f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left
        -0.53f, 1.31f, -0.14f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Top Left (Cut in Front)
        -0.26f, 1.51f, -0.5f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Point

        // Left
        -1.51f, 1.31f, -0.13f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Left
        -1.51f, 1.31f, -1.01f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Back Left
        -1.24f, 1.51f, -0.13f,       0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Point
        -0.97f, 1.31f, -0.13f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Right
        -0.97f, 1.31f, -0.14f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Right (Cut in Front)
        -1.24f, 1.51f, -0.5f,        0.0f, 0.0f, -1.0f,       1.0f, 1.0f, // Point
    };

    GLushort topRoofIndices[] = {
        0,1,2,
        0,2,5,
        2,5,1,
        5,4,2,
        4,3,2,
        1,7,5,
        7,11,5,
        11,10,5,
        10,4,5,
        10,9,8,
        8,10,11,
        11,8,7,
        7,6,8
    };

    glGenVertexArrays(1, &mesh.vao8); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao8);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos8);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos8[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(topRoof), topRoof, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nTopRoofIndices = sizeof(topRoofIndices) / sizeof(topRoofIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos8[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(topRoofIndices), topRoofIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //==========================================================================================================================================
        // Top Windows

    GLfloat topWindows[] = {
        // Right Window
        -0.04f, 1.2f, -0.14f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        -0.04f, 0.9f, -0.14f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        -0.48f, 0.9f, -0.14f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        -0.48f, 1.2f, -0.14f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left

        //Left Window
        -1.46f, 1.2f, -0.14f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        -1.46f, 0.9f, -0.14f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        -1.02f, 0.9f, -0.14f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        -1.02f, 1.2f, -0.14f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left
    };

    GLushort topWindowIndices[] = {
        0,1,2,
        2,0,3,
        3,2,1,

        4,5,6,
        6,4,7,
        7,6,5
    };

    glGenVertexArrays(1, &mesh.vao9); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao9);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos9);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos9[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(topWindows), topWindows, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nWindowIndices = sizeof(topWindowIndices) / sizeof(topWindowIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos9[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(topWindowIndices), topWindowIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //============================================================================================================================================
        //Front Step

    GLfloat frontStep[] = {
        // Front Face
        -0.5f, -0.2f, 0.0f,     0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right - Front Step
        -0.5f, -0.1f, 0.0f,      0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right - Front Step
        -1.5f, -0.1f, 0.0f,      0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Top Left - Front Step
        -1.5f, -0.2f, 0.0f,      0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Bottom Left - Front Step

        // Back Face
        -0.5f, -0.2f, -0.2f,     0.0f, 0.0f, -1.0f,      1.0f, 0.0f, // Bottom Right
        -0.5f, -0.1f, -0.2f,     0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        -1.5f, -0.1f, -0.2f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Top left
        -1.5f, -0.2f, -0.2f,    0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Bottom Left - Front Step

        // Side Walk
        -0.9f, -0.19f, 0.0f,     0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
        -0.7f, -0.19f, 0.0f,     0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
        -0.9f, -0.19f, 0.3f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
        -0.7f, -0.19f, 0.1f,     0.0f, 0.0f, 1.0f,       0.0f, 1.0f,
        -0.3f, -0.19f, 0.1f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f,
        -0.3f, -0.19f, 0.3f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f
    };

    GLushort frontStepIndices[] = {
        0,1,2,
        2,0,3,
        3,2,7,
        7,6,2,
        2,3,7,
        7,4,0,
        1,5,6,
        6,1,2,
        2,6,5,

        8,9,10,
        10,11,9,
        9,8,10,
        10,13,11,
        11,12,13
    };

    glGenVertexArrays(1, &mesh.vao10); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao10);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos10);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos10[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(frontStep), frontStep, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nWalkUpIndices = sizeof(frontStepIndices) / sizeof(frontStepIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos10[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontStepIndices), frontStepIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //====================================================================================================================================================
        //Front Door

    GLfloat frontDoor[] = {
        -0.9f, -0.1f, -0.19f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        -0.9f, 0.35f, -0.19f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        -0.7f, 0.35f, -0.19f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        -0.7f, -0.1f, -0.19f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left
    };

    GLushort frontDoorIndices[] = {
        0,1,2,
        2,0,3,
        3,2,1
    };

    glGenVertexArrays(1, &mesh.vao11); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao11);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos11);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos11[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(frontDoor), frontDoor, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nFrontDoorIndices = sizeof(frontDoorIndices) / sizeof(frontDoorIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos11[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontDoorIndices), frontDoorIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

    //=====================================================================================================================================
        //Garage

    GLfloat garage[] = {
        0.3f,  0.4f, 0.01f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f, // Top Right (Front) - Vertex 0
         0.3f, -0.2f, 0.01f,   0.0f, 0.0f, 1.0f,    1.0f, 0.0f, // Bottom Right (Front) - Vertex 1
        -0.3f, -0.2f, 0.01f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f, // Bottom Left (Front) - Vertex 2
        -0.3f,  0.4f, 0.01f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f, // Top Left (Front) - Vertex 3
    };

    GLushort garageIndices[] = {
        0,1,2,
        2,0,3,
        3,2,1
    };

    glGenVertexArrays(1, &mesh.vao12); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao12);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos12);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos12[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(garage), garage, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nGarageIndices = sizeof(garageIndices) / sizeof(garageIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos12[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(garageIndices), garageIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

//=========================================================================================================================================
    // Office Window

    GLfloat frontWindow[] = {
        // Right Window
        -1.05f, 0.4f, -0.19f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        -1.05f, 0.1f, -0.19f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        -1.2f, 0.1f, -0.19f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        -1.2f, 0.4f, -0.19f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left

        //Left Window
        -1.3f, 0.4f, -0.19f,        0.0f, 0.0f, 1.0f,       1.0f, 1.0f, // Top Right
        -1.3f, 0.1f, -0.19f,        0.0f, 0.0f, 1.0f,       1.0f, 0.0f, // Bottom Right
        -1.45f, 0.1f, -0.19f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, // Bottom Left
        -1.45f, 0.4f, -0.19f,       0.0f, 0.0f, 1.0f,       0.0f, 1.0f, // Top Left
    };

    GLushort frontWindowIndices[] = {
        0,1,2,
        2,0,3,
        3,2,1,

        4,5,6,
        6,4,7,
        7,6,5
    };

    glGenVertexArrays(1, &mesh.vao13); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao13);

    // Create 2 buffers: first one for the vertex data; second one for the indices
    glGenBuffers(2, mesh.vbos13);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos13[0]); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(frontWindow), frontWindow, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    mesh.nFrontWindowIndices = sizeof(frontWindowIndices) / sizeof(frontWindowIndices[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos13[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(frontWindowIndices), frontWindowIndices, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);

//==========================================================================================================================================
    // Fence

    //GLfloat fence[] = {
    //    0.5f, -0.2f, -0.3f,     0.0f, 0.0f, 1.0f,       1.0f, 1.0f, //0
    //    0.5f, 0.3f, -0.3f,     0.0f, 0.0f, 1.0f,       1.0f, 0.0f, //1
    //    1.5f, 0.3f, -0.3f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f, //2
    //    1.5f, -0.2f, -0.3f,     0.0f, 0.0f, 1.0f,       0.0f, 1.0f,//3
    //    1.5f, -0.2f, -2.0f,     0.0f, 0.0f, -1.0f,       1.0f, 1.0f,//4
    //    1.5f, 0.3f, -2.0f,     0.0f, 0.0f, -1.0f,       1.0f, 0.0f,//5
    //    -2.0f, -0.2f, -2.0f,     0.0f, 0.0f, -1.0f,       0.0f, 0.0f,//6
    //    -2.0f, 0.3f, -0.3f,     0.0f, 0.0f, -1.0f,       0.0f, 1.0f,//7
    //    -2.0f, -0.2f, -0.5f,     0.0f, 0.0f, 1.0f,       1.0f, 1.0f,//8
    //    -2.0f, 0.3f, -0.5f,     0.0f, 0.0f, 1.0f,       1.0f, 0.0f,//9
    //    -1.5f, -0.2f, -0.5f,     0.0f, 0.0f, 1.0f,       0.0f, 0.0f,//10
    //    -1.5f, 0.3f, -0.5f,     0.0f, 0.0f, 1.0f,       0.0f, 1.0f,//11
    //};

    //GLushort fenceIndices[] = {
    //    0,1,3,
    //    0,3,2,
    //    1,2,3,
    //    3,4,2,
    //    2,5,4,
    //    4,6,5,
    //    6,7,5,
    //    6,8,7,
    //    8,9,7,
    //    8,10,11,
    //    11,8,9,
    //    9,11,10
    //};

    //glGenVertexArrays(1, &mesh.vao14); // we can also generate multiple VAOs or buffers at the same time
    //glBindVertexArray(mesh.vao14);

    //// Create 2 buffers: first one for the vertex data; second one for the indices
    //glGenBuffers(2, mesh.vbos14);
    //glBindBuffer(GL_ARRAY_BUFFER, mesh.vbos14[0]); // Activates the buffer
    //glBufferData(GL_ARRAY_BUFFER, sizeof(fence), fence, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    //mesh.nFenceIndices = sizeof(fenceIndices) / sizeof(fenceIndices[0]);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos14[1]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(fenceIndices), fenceIndices, GL_STATIC_DRAW);

    //// Create Vertex Attribute Pointers
    //glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    //glEnableVertexAttribArray(0);

    //glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* floatsPerVertex));
    //glEnableVertexAttribArray(1);

    //glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float)* (floatsPerVertex + floatsPerNormal)));
    //glEnableVertexAttribArray(2);
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(2, mesh.vbos);
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }


    // Error loading the image
    return false;
}

void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}

