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

    const Matrix4D bodyScale = Matrix4D::scale(3.5f, 0.9f, 1.25f);
    const Matrix4D bodyTrans = Matrix4D::translation({0.0f, 0.0f, 0.0f});

    const Matrix4D mastScale = Matrix4D::scale(0.15f, 1.5f, 0.15f);
    const Matrix4D mastTrans = Matrix4D::translation({-1.0f, 2.4f, 0.0f});

    const Matrix4D bridgeScale = Matrix4D::scale(0.65f, 0.75f, 0.75f);
    const Matrix4D bridgeTrans = Matrix4D::translation({1.5f, 1.65f, 0});

    const Matrix4D bulwarkLeftScale = Matrix4D::scale(3.2f, 0.3f, 0.15f);
    const Matrix4D bulwarkLeftTrans = Matrix4D::translation({0, 1.2f, -1.1f});

    const Matrix4D bulwarkRightScale = Matrix4D::scale(3.2f, 0.3f, 0.15f);
    const Matrix4D bulwarkRightTrans = Matrix4D::translation({0, 1.2f, 1.1f});

    const Matrix4D bulwarkFrontScale = Matrix4D::scale(0.15f, 0.3f, 1.25f);
    const Matrix4D bulwarkFrontTrans = Matrix4D::translation({3.35f, 1.2f, 0});

    const Matrix4D bulwarkBackScale = Matrix4D::scale(0.15f, 0.3f, 1.25f);
    const Matrix4D bulwarkBackTrans = Matrix4D::translation({-3.35f, 1.2f, 0});
}



// Position of the central point of each cube before any transformation (with homogenuous
    // coordinates):
    const Vector4D centralPointBeforeTransformation = { 0.0, 0.0, 0.0, 1.0 };

    // Colors:
    const Vector4D colorBlack = { 0.0f, 0.0f, 0.0f, 1.0f };
    const Vector4D colorLightYellow = { 1.0f, 0.88f, 0.0f, 1.0f };
    const Vector4D colorDarkYellow = { 0.78f, 0.55f, 0.0f, 1.0f };


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
    Mesh cubeMesh;
    Matrix4D cubeScalingMatrix;
    Matrix4D cubeTranslationMatrix;
    Matrix4D cubeTransformationMatrix;
    float cubeSpinRadPerSecond;
    /* boat mesh and transformations */
    Mesh bodyMesh;
    Mesh mastMesh;
    Mesh bridgeMesh;
    Mesh bulwarkLeftMesh;
    Mesh bulwarkRightMesh;
    Mesh bulwarkFrontMesh;
    Mesh bulwarkBackMesh;

    Matrix4D bodyTransformationMatrix;

    Matrix4D bodyScalingMatrix;
    Matrix4D bodyTranslationMatrix;

    Matrix4D mastScalingMatrix;
    Matrix4D mastTranslationMatrix;
    Matrix4D mastTransformationMatrix;

    Matrix4D bridgeScalingMatrix;
    Matrix4D bridgeTranslationMatrix;
    Matrix4D bridgeTransformationMatrix;

    Matrix4D bulwarkLeftScalingMatrix;
    Matrix4D bulwarkLeftTranslationMatrix;
    Matrix4D bulwarkLeftTransformationMatrix;

    Matrix4D bulwarkRightScalingMatrix;
    Matrix4D bulwarkRightTranslationMatrix;
    Matrix4D bulwarkRightTransformationMatrix;

    Matrix4D bulwarkFrontScalingMatrix;
    Matrix4D bulwarkFrontTranslationMatrix;
    Matrix4D bulwarkFrontTransformationMatrix;

    Matrix4D bulwarkBackScalingMatrix;
    Matrix4D bulwarkBackTranslationMatrix;
    Matrix4D bulwarkBackTransformationMatrix;



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
void setupBoat() {
    sScene.bodyScalingMatrix = boat::bodyScale;
    sScene.bodyTranslationMatrix = boat::bodyTrans;
    sScene.bodyTransformationMatrix = Matrix4D::identity();

    sScene.mastScalingMatrix = boat::mastScale;
    sScene.mastTranslationMatrix = boat::mastTrans;
    sScene.mastTransformationMatrix = Matrix4D::identity();

    sScene.bridgeScalingMatrix = boat::bridgeScale;
    sScene.bridgeTranslationMatrix = boat::bridgeTrans;
    sScene.bridgeTransformationMatrix = Matrix4D::identity();

    sScene.bulwarkLeftScalingMatrix = boat::bulwarkLeftScale;
    sScene.bulwarkLeftTranslationMatrix = boat::bulwarkLeftTrans;
    sScene.bulwarkLeftTransformationMatrix = Matrix4D::identity();

    sScene.bulwarkRightScalingMatrix = boat::bulwarkRightScale;
    sScene.bulwarkRightTranslationMatrix = boat::bulwarkRightTrans;
    sScene.bulwarkRightTransformationMatrix = Matrix4D::identity();

    sScene.bulwarkFrontScalingMatrix = boat::bulwarkFrontScale;
    sScene.bulwarkFrontTranslationMatrix = boat::bulwarkFrontTrans;
    sScene.bulwarkFrontTransformationMatrix = Matrix4D::identity();

    sScene.bulwarkBackScalingMatrix = boat::bulwarkBackScale;
    sScene.bulwarkBackTranslationMatrix = boat::bulwarkBackTrans;
    sScene.bulwarkBackTransformationMatrix = Matrix4D::identity();
}
/* function to setup and initialize the whole scene */
void sceneInit(float width, float height)
{
    /* initialize camera[0] */
    sScene.cameras[0] = cameraCreate(width, height, to_radians(45.0f), 0.01f, 500.0f, {10.0f, 14.0f, 10.0f}, {0.0f, 4.0f, 0.0f});
    sScene.zoomSpeedMultiplier = 0.05f;

    /* setup objects in scene and create opengl buffers for meshes */
    sScene.cubeMesh = meshCreate(cube::vertices, cube::indices, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.water = waterCreate(waterPlane::color);


    /* setup transformation matrices for objects */
    sScene.waterModelMatrix = waterPlane::trans;


    sScene.bodyMesh = meshCreate(cube::vertexPos, cube::indices, {0.5f, 0.102f, 0, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.mastMesh = meshCreate(cube::vertexPos, cube::indices, {0.3f, 0.102f, 0, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.bulwarkBackMesh = meshCreate(cube::vertexPos, cube::indices, {0.75f, 0.4f, 0, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.bulwarkLeftMesh = meshCreate(cube::vertexPos, cube::indices, {0.75f, 0.4f, 0, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.bulwarkFrontMesh = meshCreate(cube::vertexPos, cube::indices, {0.75f, 0.4f, 0, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.bulwarkRightMesh = meshCreate(cube::vertexPos, cube::indices, {0.75f, 0.4f, 0, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);
    sScene.bridgeMesh = meshCreate(cube::vertexPos, cube::indices, {1.0f, 1.0f, 1.0f, 1.0f}, GL_STATIC_DRAW, GL_STATIC_DRAW);

    setupBoat();
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
void sceneUpdate(float dt) {
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
    if (sInput.buttonPressed[4] && sScene.currentCamera == 1) {
        // Change to camera mode 1.
        cameraChange = true;
        newCamera = 0;
    } else if (sInput.buttonPressed[5] && sScene.currentCamera == 0) {
        // Change to camera mode 2.
        cameraChange = true;
        newCamera = 1;
    }
//    Mesh bodyMesh;
//    Mesh mastMesh;
//    Mesh bridgeMesh;
//    Mesh bulwarkLeftMesh;
//    Mesh bulwarkRightMesh;
//    Mesh bulwarkFrontMesh;
//    Mesh bulwarkBackMesh;

//    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
//    for (int i = 0; i < sizeof(sScene.cubeMesh); i++) {
//        if (rotationDirX != 0 || rotationDirY != 0) {
//            sScene.cubeTransformationMatrix[i] = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt) *
//                                                 Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) *
//                                                 sScene.cubeTransformationMatrix[i];
//        }
//    }
    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
    if (rotationDirX != 0 || rotationDirY != 0) {
        sScene.bodyTransformationMatrix = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt)
                * Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) * sScene.bodyTransformationMatrix;
    }
    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
    if (rotationDirX != 0 || rotationDirY != 0) {
        sScene.mastTransformationMatrix = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt)
                                          * Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) * sScene.mastTransformationMatrix;
    }
    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
    if (rotationDirX != 0 || rotationDirY != 0) {
        sScene.bridgeTransformationMatrix = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt)
                                          * Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) * sScene.bridgeTransformationMatrix;
    }
    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
    if (rotationDirX != 0 || rotationDirY != 0) {
        sScene.bulwarkFrontTransformationMatrix = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt)
                                          * Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) * sScene.bulwarkFrontTransformationMatrix;
    }
    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
    if (rotationDirX != 0 || rotationDirY != 0) {
        sScene.bulwarkBackTransformationMatrix = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt)
                                                  * Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) * sScene.bulwarkBackTransformationMatrix;
    }
    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
    if (rotationDirX != 0 || rotationDirY != 0) {
        sScene.bulwarkLeftTransformationMatrix = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt)
                                                  * Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) * sScene.bulwarkLeftTransformationMatrix;
    }
    /* udpate cube transformation matrix to include new rotation if one of the keys was pressed */
    if (rotationDirX != 0 || rotationDirY != 0) {
        sScene.bulwarkRightTransformationMatrix = Matrix4D::rotationY(rotationDirY * sScene.cubeSpinRadPerSecond * dt)
                                                  * Matrix4D::rotationX(rotationDirX * sScene.cubeSpinRadPerSecond * dt) * sScene.bulwarkLeftTransformationMatrix;
    }
        // Update camera:
        Vector3D centralPointOfBoat =
                vector4dToVector3d((sScene.bodyTransformationMatrix) * centralPointBeforeTransformation);
        if (cameraChange) {
            Camera oldCamera = sScene.cameras[sScene.currentCamera];
            if (newCamera == 0) {
                sScene.cameras[newCamera] = cameraCreate(oldCamera.width, oldCamera.height, oldCamera.fov,
                                                         oldCamera.nearPlane,
                                                         oldCamera.farPlane, oldCamera.position, oldCamera.lookAt);
            } else { // newCamera == 1
                sScene.cameras[newCamera] = cameraCreate(oldCamera.width, oldCamera.height, oldCamera.fov,
                                                         oldCamera.nearPlane,
                                                         oldCamera.farPlane, oldCamera.position, centralPointOfBoat);
            }

            sScene.currentCamera = newCamera;
        }

        if (sScene.currentCamera == 1) {
            sScene.cameras[sScene.currentCamera].lookAt = centralPointOfBoat;
        }
    }

void boatDraw()
{
    shaderUniform(sScene.shaderColor, "uModel", sScene.bodyTranslationMatrix * sScene.bodyTransformationMatrix * sScene.bodyScalingMatrix);
    glBindVertexArray(sScene.bodyMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.bodyMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    shaderUniform(sScene.shaderColor, "uModel", sScene.bodyTransformationMatrix * sScene.mastTranslationMatrix * sScene.mastScalingMatrix);
    glBindVertexArray(sScene.mastMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.mastMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    shaderUniform(sScene.shaderColor, "uModel",sScene.bodyTransformationMatrix * sScene.bridgeTranslationMatrix * sScene.bridgeScalingMatrix);
    glBindVertexArray(sScene.bridgeMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.bridgeMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    shaderUniform(sScene.shaderColor, "uModel", sScene.bodyTransformationMatrix * sScene.bulwarkLeftTranslationMatrix * sScene.bulwarkLeftScalingMatrix);
    glBindVertexArray(sScene.bulwarkLeftMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.bulwarkLeftMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    shaderUniform(sScene.shaderColor, "uModel",  sScene.bodyTransformationMatrix * sScene.bulwarkBackTranslationMatrix * sScene.bulwarkBackScalingMatrix);
    glBindVertexArray(sScene.bulwarkBackMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.bulwarkBackMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    shaderUniform(sScene.shaderColor, "uModel",  sScene.bodyTransformationMatrix * sScene.bulwarkRightTranslationMatrix * sScene.bulwarkRightScalingMatrix);
    glBindVertexArray(sScene.bulwarkRightMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.bulwarkRightMesh.size_ibo, GL_UNSIGNED_INT, nullptr);

    shaderUniform(sScene.shaderColor, "uModel",  sScene.bodyTransformationMatrix * sScene.bulwarkFrontTranslationMatrix * sScene.bulwarkFrontScalingMatrix);
    glBindVertexArray(sScene.bulwarkFrontMesh.vao);
    glDrawElements(GL_TRIANGLES, sScene.bulwarkFrontMesh.size_ibo, GL_UNSIGNED_INT, nullptr);
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
        glBindVertexArray(sScene.cubeMesh.vao);
        glDrawElements(GL_TRIANGLES, sScene.cubeMesh.size_ibo, GL_UNSIGNED_INT, nullptr);
        }
        boatDraw();
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
    meshDelete(sScene.cubeMesh);

    /* cleanup glfw/glcontext */
    windowDelete(window);

    return EXIT_SUCCESS;
}
