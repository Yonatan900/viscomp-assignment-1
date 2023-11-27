#include <cstdlib>
#include <iostream>

#include "mygl/shader.h"
#include "mygl/mesh.h"
#include "mygl/geometry.h"
#include "mygl/camera.h"
#include "water.h"

/* translation and color for the water plane */
namespace waterPlane
{const Vector4D color = {0.0f, 0.0f, 0.35f, 1.0f};
const Matrix4D scale = Matrix4D::scale(50.0f, 0.0f, 50.0f);
const Matrix4D trans = Matrix4D::identity();
}

/* translation and scale for the scaled cube */
namespace boat {
    const Matrix4D scale = Matrix4D::scale(2.0f, 2.0f, 2.0f);
    const Matrix4D trans = Matrix4D::translation({0.0f, 4.0f, 0.0f});

    // Scales:
    const Matrix4D scaleMast = Matrix4D::scale(0.1f, 0.1f, 1.0f);
    const Matrix4D scaleCube = Matrix4D::scale(0.4f, 0.4f, 0.4f);
    const Matrix4D scaleBody = Matrix4D::scale(2.3f, 0.4f, 0.9f);
    const Matrix4D scaleTop = Matrix4D::scale(2.3f, 0.4f, 0.9f);
// Initial translations:
    const Matrix4D initialTranslations[] = {
            Matrix4D::translation({1.6f, 0.3f, -1.0125f}),  // transTireFL
            Matrix4D::translation({1.6f, 0.3f, 1.0125f}),   // transTireFR
            Matrix4D::translation({-1.4f, 0.3f, -1.0125f}), // transTireRL
            Matrix4D::translation({-1.4f, 0.3f, 1.0125f}),  // transTireRR
            Matrix4D::translation({-2.4125f, 1.1f, 0.0f}),  // transTireRes
            Matrix4D::translation({1.6f, 0.3f, 0.0f}),      // transAxisF
            Matrix4D::translation({-1.4f, 0.3f, 0.0f}),     // transAxisR
    };
// Position of the central point of each cube before any transformation (with homogenuous
    // coordinates):
    const Vector4D centralPointBeforeTransformation = { 0.0, 0.0, 0.0, 1.0 };

    // Colors:
    const Vector4D colorBlack = { 0.0f, 0.0f, 0.0f, 1.0f };
    const Vector4D colorLightYellow = { 1.0f, 0.88f, 0.0f, 1.0f };
    const Vector4D colorDarkYellow = { 0.78f, 0.55f, 0.0f, 1.0f };
    // Constants:
    const int numCubes = 7;
    const int WHEELS = 3;
    const int WHEELS_ALL = 4; // Includes reserver wheel.
    const int WHEEL_FL = 0;
    const int WHEEL_FR = 1;
    const int WHEEL_RL = 2;
    const int WHEEL_RR = 3;
    const int WHEEL_RES = 4;
    const int AXIS_F = 5;
    const int AXIS_R = 6;
    const int BODY = 7;
    const int TOP = 8;
}
/* struct holding all necessary state variables for scene */
struct
{
    /* camera */
    Camera cameras[2];
    short currentCamera;
    float zoomSpeedMultiplier;

    /* water */
    WaterSim waterSim;
    Water water;
    Matrix4D waterModelMatrix;

    /* cube mesh and transformations */
    Mesh cubeMesh[boat::numCubes];
    Matrix4D cubeScalingMatrix;
    Matrix4D cubeTranslationMatrix;
    Matrix4D cubeTransformationMatrix;
    float cubeSpinRadPerSecond;
    Matrix4D cubeModelMatrix[boat::numCubes];


    /* shader */
    ShaderProgram shaderColor;
} sScene;

/* struct holding all state variables for input */
struct
{
    bool mouseLeftButtonPressed = false;
    Vector2D mousePressStart;
    bool buttonPressed[6] = {false, false, false, false, false, false};
} sInput;

/* GLFW callback function for keyboard events */
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    /* called on keyboard event */

    /* close window on escape */
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    /* make screenshot and save in work directory */
    if(key == GLFW_KEY_P && action == GLFW_PRESS)
    {
        screenshotToPNG("screenshot.png");
    }

    /* input for cube control */
    if(key == GLFW_KEY_W)
    {
        sInput.buttonPressed[0] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if(key == GLFW_KEY_S)
    {
        sInput.buttonPressed[1] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    if(key == GLFW_KEY_A)
    {
        sInput.buttonPressed[2] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if(key == GLFW_KEY_D)
    {
        sInput.buttonPressed[3] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if(key == GLFW_KEY_1) {
        sInput.buttonPressed[4] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if(key == GLFW_KEY_2) {
        sInput.buttonPressed[5] = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
}

/* GLFW callback function for mouse position events */
void mousePosCallback(GLFWwindow* window, double x, double y)
{
    /* called on cursor position change */
    if(sInput.mouseLeftButtonPressed)
    {
        Vector2D diff = sInput.mousePressStart - Vector2D(x, y);
        cameraUpdateOrbit(sScene.cameras[sScene.currentCamera], diff, 0.0f);
        sInput.mousePressStart = Vector2D(x, y);
    }
}

/* GLFW callback function for mouse button events */
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        sInput.mouseLeftButtonPressed = (action == GLFW_PRESS || action == GLFW_REPEAT);

        double x, y;
        glfwGetCursorPos(window, &x, &y);
        sInput.mousePressStart = Vector2D(x, y);
    }
}

/* GLFW callback function for mouse scroll events */
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    cameraUpdateOrbit(sScene.cameras[sScene.currentCamera], {0, 0}, sScene.zoomSpeedMultiplier * yoffset);
}

/* GLFW callback function for window resize event */
void windowResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    sScene.cameras[sScene.currentCamera].width = width;
    sScene.cameras[sScene.currentCamera].height = height;
}
/* function to setup and initialize the whole scene */
void sceneInit(float width, float height)
{
    /* initialize camera[0] */
    sScene.cameras[0] = cameraCreate(width, height, to_radians(45.0f), 0.01f, 500.0f, {10.0f, 14.0f, 10.0f}, {0.0f, 4.0f, 0.0f});
    sScene.zoomSpeedMultiplier = 0.05f;

    /* setup objects in scene and create opengl buffers for meshes */
//    sScene.cubeMesh = meshCreate(cube::vertices, cube::indices, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.water = waterCreate(waterPlane::color);

    for(int i = 0; i < boat::numCubes; i++) {
        sScene.cubeMesh[i] = meshCreate(cube::vertices, cube::indices,
//                                      i <= 4 ? boat::colorBlack
//                                                           : i <= 5? boat::colorDarkYellow
//                                                                                : boat::colorLightYellow,
                                                                                GL_STATIC_DRAW, GL_STATIC_DRAW);
    }



    /* create 7 init cubes*/
    for(int i = 0; i < boat::numCubes; i++) {
        sScene.cubeModelMatrix[i] = Matrix4D::identity();
    }

    // Scale all unit cubes:
    // Scale wheels:
    for(int i = 0; i <= boat::WHEELS_ALL; i++) {
        sScene.cubeModelMatrix[i] = boat::scaleMast * sScene.cubeModelMatrix[i];
    }

    // Scales axises:
    for(int i = boat::AXIS_F; i <= boat::AXIS_R; i++) {
        sScene.cubeModelMatrix[i] = boat::scaleCube * sScene.cubeModelMatrix[i];
    }

    // Scale body and top:
    sScene.cubeModelMatrix[boat::BODY] = boat::scaleBody * sScene.cubeModelMatrix[boat::BODY];
    sScene.cubeModelMatrix[boat::TOP] = boat::scaleTop * sScene.cubeModelMatrix[boat::TOP];

    // Translate all cubes:
    for(int i = 0; i < boat::numCubes; i++) {
        sScene.cubeModelMatrix[i] = boat::initialTranslations[i] * sScene.cubeModelMatrix[i];
    }

    /* setup transformation matrices for objects */
    sScene.waterModelMatrix = waterPlane::trans;

    sScene.cubeScalingMatrix = boat::scale;
    sScene.cubeTranslationMatrix = boat::trans;

    sScene.cubeTransformationMatrix = Matrix4D::identity();

    sScene.cubeSpinRadPerSecond = M_PI / 2.0f;

    /* load shader from file */
    sScene.shaderColor = shaderLoad("shader/default.vert", "shader/default.frag");
}
/* function to setup and initialize the whole scene */
//void sceneInit(float width, float height) {
//    /* initialize camera */
//    sScene.camera = cameraCreate(width, height, to_radians(45.0f), 0.01f, 500.0f, {10.0f, 14.0f, 10.0f},
//                                 {0.0f, 4.0f, 0.0f});
//    sScene.zoomSpeedMultiplier = 0.05f;
//
//    /* setup objects in scene and create opengl buffers for meshes */
//    for (int i = 0; i < 7; i++) {
//        sScene.cubeMesh[i] = meshCreate(cube::vertices, cube::indices, GL_STATIC_DRAW, GL_STATIC_DRAW);
//        //push these arrays up.
//        // need to insert correct values in these arrays.
//        float cubeScaleValues[7][3]={{1.0f, 0.4f, 2.0f},
//                                     {0.075f, 0.5f, 1.0f},
//                                     {0.0f, 0.0f, 0.0f},
//                                     {0.0f, 0.0f, 0.0f},
//                                     {0.0f, 0.0f, 0.0f},
//                                     {0.0f, 0.0f, 0.0f},
//                                     {0.0f, 0.0f, 0.0f}};
//        std::vector<Vector3D> cubePosValues = { {0.0f, 0.0f, 0.0f},
//                                                {2.0f, 0.5f, 0.0f},
//                                                {0.0f, 0.0f, 0.0f},
//                                                {0.0f, 0.0f, 0.0f},
//                                                {0.0f, 0.0f, 0.0f},
//                                                {0.0f, 0.0f, 0.0f},
//                                                {0.0f, 0.0f, 0.0f},
//                                                {0.0f, 0.0f, 0.0f} };
//
//                sScene.cubeScalingMatrix[i] = Matrix4D::scale(cubeScaleValues[i][0],cubeScaleValues[i][1],cubeScaleValues[i][2]);
//                sScene.cubeTranslationMatrix[i] = Matrix4D::translation(cubePosValues[i]);
//                sScene.cubeTransformationMatrix[i] =  sScene.cubeScalingMatrix[i]*sScene.cubeTranslationMatrix[i];
//                sScene.cubeSpinRadPerSecond[i] = M_PI / 2.0f;
//
//    }
//
//    sScene.water = waterCreate(waterPlane::color);
//
//    /* setup transformation matrices for objects */
//    sScene.waterModelMatrix = waterPlane::trans;
//
//
//
//    /* load shader from file */
//    sScene.shaderColor = shaderLoad("shader/default.vert", "shader/default.frag");
//}
// Helper functions:
Vector3D vector4dToVector3d(Vector4D vec4d) {
    return { vec4d[0] / vec4d[3], vec4d[1] / vec4d[3], vec4d[2] / vec4d[3] };
}
/* function to move and update objects in scene (e.g., rotate cube according to user input) */
void sceneUpdate(float dt)
{
    /* if 'w' or 's' pressed, cube should rotate around x axis */
    int rotationDirX = 0;
    if (sInput.buttonPressed[0]) {
        rotationDirX = -1;
    } else if (sInput.buttonPressed[1]) {
        rotationDirX = 1;
    }

    /* if 'a' or 'd' pressed, cube should rotate around y axis */
    int rotationDirY = 0;
    if (sInput.buttonPressed[2]) {
        rotationDirY = -1;
    } else if (sInput.buttonPressed[3]) {
        rotationDirY = 1;
    }
    bool cameraChange = false;
    int newCamera = 0;
    if(sInput.buttonPressed[4] && sScene.currentCamera == 1) {
        // Change to camera mode 1.
        cameraChange = true;
        newCamera = 0;
    } else if(sInput.buttonPressed[5] && sScene.currentCamera == 0) {
        // Change to camera mode 2.
        cameraChange = true;
        newCamera = 1;
    }
    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
    for (int i = 0; i < sizeof(sScene.cubeMesh); i++) {
        if (rotationDirX != 0 || rotationDirY != 0) {
            sScene.cubeTransformationMatrix[i] = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt) *
                                              Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) *
                                              sScene.cubeTransformationMatrix[i];
        }
    }

    // Update camera:
    Vector3D centralPointOfBoat =
            vector4dToVector3d(sScene.cubeModelMatrix[boat::BODY] * boat::centralPointBeforeTransformation);
    if(cameraChange) {
        Camera oldCamera = sScene.cameras[sScene.currentCamera];
        if(newCamera == 0) {
            sScene.cameras[newCamera] = cameraCreate(oldCamera.width, oldCamera.height, oldCamera.fov, oldCamera.nearPlane,
                                                     oldCamera.farPlane, oldCamera.position, oldCamera.lookAt);
        } else { // newCamera == 1
            sScene.cameras[newCamera] = cameraCreate(oldCamera.width, oldCamera.height, oldCamera.fov, oldCamera.nearPlane,
                                                     oldCamera.farPlane, oldCamera.position, centralPointOfBoat);
        }

        sScene.currentCamera = newCamera;
    }

    if(sScene.currentCamera == 1) {
        sScene.cameras[sScene.currentCamera].lookAt = centralPointOfBoat;
    }
}





/* function to draw all objects in the scene */
void sceneDraw()
{
    /* clear framebuffer color */
    glClearColor(135.0 / 255, 206.0 / 255, 235.0 / 255, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*------------ render scene -------------*/
    /* use shader and set the uniforms (names match the ones in the shader) */
    {
        glUseProgram(sScene.shaderColor.id);
        shaderUniform(sScene.shaderColor, "uProj",  cameraProjection(sScene.cameras[sScene.currentCamera]));
        shaderUniform(sScene.shaderColor, "uView",  cameraView(sScene.cameras[sScene.currentCamera]));

        /* draw water plane */
        shaderUniform(sScene.shaderColor, "uModel", sScene.waterModelMatrix);
        glBindVertexArray(sScene.water.mesh.vao);
        glDrawElements(GL_TRIANGLES, sScene.water.mesh.size_ibo, GL_UNSIGNED_INT, nullptr);

        /* draw cube, requires to calculate the final model matrix from all transformations */
        for (int i = 0; i < 7; i++){
        shaderUniform(sScene.shaderColor, "uModel", sScene.cubeTranslationMatrix * sScene.cubeTransformationMatrix * sScene.cubeScalingMatrix);
        glBindVertexArray(sScene.cubeMesh[i].vao);
        glDrawElements(GL_TRIANGLES, sScene.cubeMesh[i].size_ibo, GL_UNSIGNED_INT, nullptr);
        }
    }
    glCheckError();

    /* cleanup opengl state */
    glBindVertexArray(0);
    glUseProgram(0);
}

int main(int argc, char** argv)
{
    /* create window/context */
    int width = 1280;
    int height = 720;
    GLFWwindow* window = windowCreate("Assignment 1 - Transformations, User Input and Camera", width, height);
    if(!window) { return EXIT_FAILURE; }

    /* set window callbacks */
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mousePosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, mouseScrollCallback);
    glfwSetFramebufferSizeCallback(window, windowResizeCallback);


    /*---------- init opengl stuff ------------*/
    glEnable(GL_DEPTH_TEST);

    /* setup scene */
    sceneInit(width, height);

    /*-------------- main loop ----------------*/
    double timeStamp = glfwGetTime();
    double timeStampNew = 0.0;

    /* loop until user closes window */
    while(!glfwWindowShouldClose(window))
    {
        /* poll and process input and window events */
        glfwPollEvents();

        /* update model matrix of cube */
        timeStampNew = glfwGetTime();
        sceneUpdate(timeStampNew - timeStamp);
        timeStamp = timeStampNew;

        /* draw all objects in the scene */
        sceneDraw();

        /* swap front and back buffer */
        glfwSwapBuffers(window);
    }


    /*-------- cleanup --------*/
    /* delete opengl shader and buffers */
    shaderDelete(sScene.shaderColor);
    waterDelete(sScene.water);
    for(int i = 0; i < boat::numCubes; i++) {
        meshDelete(sScene.cubeMesh[i]);
    }

    /* cleanup glfw/glcontext */
    windowDelete(window);

    return EXIT_SUCCESS;
}
