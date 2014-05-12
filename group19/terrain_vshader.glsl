#version 330 core

// Transformation matrices from model space to camera clip space.
uniform mat4 projection;
uniform mat4 modelview;

// Transformation matrix from model space to light clip space.
uniform mat4 lightOffsetMVP;

uniform vec3 light_dir_tmp;

// Time for water animation.
uniform float time;

// Texture 0. Defined by glActiveTexture and passed by glUniform1i.
uniform sampler2D heightMapTex;

// Vertices 2D position in model space.
// First input buffer. Defined here, retrieved in C++ by glGetAttribLocation.
layout(location = 0) in vec2 vertexPosition2DModel;

// Vertices 3D position (after heightmap displacement) in model space.
out vec3 vertexPosition3DModel;

// Vertex position in light source clip space.
out vec4 ShadowCoord;

// Light and view directions.
out vec3 light_dir;
out vec3 view_dir;


void main() {

    // World (triangle grid) coordinates are (-1,-1) x (1,1).
    // Texture (height map) coordinates are (0,0) x (1,1).
    vec2 UV = (vertexPosition2DModel + 1.0) / 2.0;
    float height = texture2D(heightMapTex, UV).r;

    // Generate waves : modulate the water height.
    if(height <= 0.01f) {
            height = sin(-6.28*vertexPosition2DModel.x - 6.28*vertexPosition2DModel.y+time*0.07) * 0.008;
    }

    // 3D vertex position : X and Y from vertex array, Z from heightmap texture.
    vertexPosition3DModel = vec3(vertexPosition2DModel.xy, height);

    // Model matrix transforms from model space to world space.
    // View matrix transforms from world space to camera space.
    // Projection matrix transforms from camera space to clip space (homogeneous space).
    vec4 vertexPositionCamera = modelview * vec4(vertexPosition3DModel,  1.0);
    gl_Position = projection * vertexPositionCamera;

    // Vertex position in light source clip space.
    ShadowCoord = lightOffsetMVP * vec4(vertexPosition3DModel, 1.0);

    // Light and view directions.
    light_dir = light_dir_tmp;
    view_dir = vec3(vertexPositionCamera);

}
