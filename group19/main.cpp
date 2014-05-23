// Copyright (C) 2014 - LGG EPFL

#include <iostream>
#include <sstream>

#include "common.h"
#include "heightmap.h"
#include "shadowmap.h"
#include "watermap.h"
#include "skybox.h"
#include "particles_control.h"
#include "particles_render.h"
#include "terrain.h"
#include "vertices.h"
#include "vertices_quad.h"
#include "vertices_grid.h"
#include "vertices_skybox.h"
#include "waterReflection.h"
#include "vertices_bezier.h"
#include "vertices_particules.h"
#include "camera.h"


/// Screen size.
const int windowWidth(1024);
const int windowHeight(768);

/// Textures (heightmap and shadowmap) sizes.
const int textureWidth(1024);
const int textureHeight(1024);

/// SQUARE ROOT of the number of particules.
//const int nParticules(10);

/// Instanciate the rendering contexts that render to the screen.
Skybox skybox(windowWidth, windowHeight);
Terrain terrain(windowWidth, windowHeight);
Camera camera(windowWidth, windowHeight);
Watermap water(windowWidth, windowHeight);
ParticlesRender particlesRender(windowWidth, windowHeight);

/// Instanciate the rendering contexts that render to FBO.
Shadowmap shadowmap(textureWidth, textureHeight);
ParticlesControl particlesControl(10, 10);

WaterReflection reflection(windowWidth, windowHeight);

/// Instanciate the vertices.
Vertices* verticesGrid = new VerticesGrid();
Vertices* verticesSkybox = new VerticesSkybox();
Vertices* verticesBezier = new VerticesBezier();
Vertices* verticesParticules = new VerticesParticules();

/// Matrices that have to be shared between functions.
mat4 cameraModelview;
static mat4 lightMVP;
static vec3 lightPositionModel;
//flip the camera for reflection effect
static mat4 flippedCameraModelview;



/// Projection parameters.
// Horizontal field of view in degrees : amount of "zoom" ("camera lens").
// Usually between 90° (extra wide) and 30° (quite zoomed in).
const float fieldOfView = 45.0f;
// Aspect ratio depends on the window size (for example 4/3 or 1).
const float aspectRatio = float(windowWidth) / float(windowHeight);
// Near clipping plane. Keep as little as possible (precision issues).
const float nearPlane = 0.1f;
// Far clipping plane. Keep as big as possible (usually 10.0f or 100.0f).
const float farPlane  = 100.0f;

/// Camera projection matrix (camera intrinsics).
const mat4 cameraProjection = Eigen::perspective(fieldOfView, aspectRatio, nearPlane, farPlane);

/// Spot light projection matrix.
const mat4 lightProjection = Eigen::perspective(fieldOfView, float(textureWidth)/float(textureHeight), nearPlane, farPlane);


void update_matrix_stack(const mat4& model) {

    /// View matrix (camera extrinsics) (position in world space).

    /// Camera is in the sky, looking down.
//    vec3 camPos(0.0f, -1.5f, 0.8f);
//    vec3 camLookAt(0.0f, 0.0f, 0.0f);
//    vec3 camUp(0.0f, 0.0f, 1.0f);
    /// Camera is right on top, comparison with light position.
    //camPos = vec3(0.0, 0.0, 5.0);
    //camLookAt = vec3(0.0, 0.0, 0.0);

    //camUp = vec3(1.0, 0.0, 0.0);
    /// Camera is in a corner, looking down to the terrain.
    //vec3 camPos(2.0f, -2.0f, 2.5f);
    //vec3 camPos(1.0f, 0.0f,0.7f); // Close texture view.

    /// View from center.


    //// Global view from outside.
    vec3 camPos(0.0f, -3.0f, 4.0f);
    vec3 camLookAt(0.0f, 0.0f, 0.0f);
    vec3 camUp(0.0f, 0.0f, 1.0f);

	//fps exploration
	//vec3 camPos(0.78f, 0.42f, 0.30f);
 //   vec3 camLookAt(-0.24f, 0.19f, 0.13f);
 //   vec3 camUp(0.0f, 0.0f, 1.0f);

    /// FPS exploration.
//	  vec3 camPos(0.78f, 0.42f, 0.30f);
//    vec3 camLookAt(-0.24f, 0.19f, 0.13f);
//    vec3 camUp(0.0f, 0.0f, 1.0f);

    /// Internal view from center.

//    vec3 camPos(0.9f, -0.8f, 1.0f);
//    vec3 camLookAt(-0.3f, 0.1f, 0.5f);
//    vec3 camUp(0.0f, 0.0f, 1.0f);

    /// Close texture view, for screenshots.
//    vec3 camPos(0.2f, -0.1f, 0.5f);
//    vec3 camLookAt(-0.3f, 0.1f, 0.2f);
//    vec3 camUp(0.0f, 0.0f, 1.0f);

    vec3 flippedCamPos = vec3(camPos[0], camPos[1],  2*GROUND_HEIGHT - camPos[2]);

    /// Assemble the view matrix.
    mat4 view = Eigen::lookAt(camPos, camLookAt, camUp);
	mat4 flippedView = Eigen::lookAt(flippedCamPos, camLookAt, camUp);

    /// Assemble the "Model View" matrix.
    cameraModelview = view * model;
	// Calculate the Model View matrix of flipped camera
	flippedCameraModelview = flippedView * model;

}


/// Key press callback.
void GLFWCALL keyboard_callback(int key, int action) {

    /// Distance from center (0,0,0) to sun.
    const float r = 3.0f;
	//std::cout << "Pressed key : " << key << std::endl;

    if(action == GLFW_PRESS) {

       // std::cout << "Pressed key : " << key << std::endl;

        /// 49 corressponds to 1, 57 to 9 (keyboard top keys).
        if(key >= 49 && key <= 57) {

            /// Angle from 0° (key 1) to 90° (key 9).
            float theta = M_PI / 8.0f * float(key-49);

            /// Position from sunrise (-r,0,0) to noon (0,0,r).
            lightPositionModel = vec3(-std::cos(theta)*r, 0.0, std::sin(theta)*r);

            /// Light source position (model coordinates).
            const vec3 lightLookAt(0.0, 0.0, 0.0);
            const vec3 lightUp(0.0, 1.0, 0.0);
            mat4 lightView = Eigen::lookAt(lightPositionModel, lightLookAt, lightUp);

            /// Assemble the lightMVP matrix for a spotlight source.
            lightMVP = lightProjection * lightView;
        }
    }
	camera.handleCameraControls(key, action);
}

void init() {
	
    /// OpenGL parameters.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_CLIP_DISTANCE0);

	//glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable (GL_BLEND);
    //glEnable(GL_CULL_FACE);

    /// Generate the heightmap texture.
    Vertices* verticesQuad = new VerticesQuad();
    verticesQuad->generate();
    Heightmap heightmap(textureWidth, textureHeight);
    GLuint heightMapTexID = heightmap.init(verticesQuad);
	heightmap.draw();
    heightmap.clean();
    verticesQuad->clean();
    delete verticesQuad;

	// write heightmap in CPU 
	camera.CopyHeightmapToCPU(heightMapTexID);
    

    /// Generate the vertices.
    verticesGrid->generate();
    verticesSkybox->generate();
    verticesBezier->generate();
    verticesParticules->generate();

    /// Initialize the rendering contexts.
    GLuint shadowMapTexID = shadowmap.init(verticesGrid, heightMapTexID);
    terrain.init(verticesGrid, heightMapTexID, shadowMapTexID);
    skybox.init(verticesSkybox);

    GLuint particlesPosTexID = particlesControl.init(NULL);
    particlesRender.init(verticesParticules, particlesPosTexID);

	water.init(verticesGrid, heightMapTexID);
	//reflection.init(verticesGrid, heightMapTexID);

    camera.init(verticesBezier);
    /// Initialize the matrix stack.  	
	update_matrix_stack(mat4::Identity());

    /// Initialize the light position.
    keyboard_callback(49, GLFW_PRESS);

}


void display() {

    /// Measure and print FPS (every second).
	static double lastTime = glfwGetTime();
    static int nbFrames = 0;
    double currentTime = glfwGetTime();
    nbFrames++;
    if(currentTime - lastTime >= 1.0) {
        std::cout << nbFrames << " FPS" << std::endl;
        nbFrames = 0;
        lastTime += 1.0;
    }

//    camera.handleCamera();
    /// Uncomment to render only the boundary (not full triangles).
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    /// Render everything.
    shadowmap.draw(lightMVP);
    terrain.draw(cameraProjection, cameraModelview, lightMVP, lightPositionModel);
    skybox.draw(cameraProjection, cameraModelview);
    camera.draw(cameraProjection, cameraModelview);
    particlesRender.draw(cameraProjection, cameraModelview);

    water.draw(cameraProjection, cameraModelview, flippedCameraModelview, lightMVP, lightPositionModel);
//    reflection.draw(cameraProjection, flippedCameraModelview, lightMVP, lightPositionModel);
   
}


int main(int, char**) {
    glfwInitWindowSize(windowWidth, windowHeight);
    glfwCreateWindow("Project - Group 19");	
    glfwDisplayFunc(display);
    init();
    glfwTrackball(update_matrix_stack);
    glfwSetKeyCallback(keyboard_callback);
    glfwMainLoop();
    return EXIT_SUCCESS;    
}
