#version 330 core

// Task 4: declare a vec3 object-space position variable, using
//         the `layout` and `in` keywords.
layout(location = 0) in vec3 objectSpacePos;
layout(location = 1) in vec3 objectSpaceNorm;

// Task 5: declare `out` variables for the world-space position and normal,
//         to be passed to the fragment shader
out vec3 worldSpacePos;
out vec3 normal;

// Task 6: declare a uniform mat4 to store model matrix
uniform mat4 modelMatrix;

// Task 7: declare uniform mat4's for the view and projection matrix
uniform mat4 viewMatrix;
uniform mat4 projMatrix;

uniform mat3 inverseTranspose;

void main() {
    // Task 8: compute the world-space position and normal, then pass them to
    //         the fragment shader using the variables created in task 5
    worldSpacePos = vec3(modelMatrix * vec4(objectSpacePos, 1.0));
    normal = normalize(inverseTranspose * objectSpaceNorm);

    // Recall that transforming normals requires obtaining the inverse-transpose of the model matrix!
    // In projects 5 and 6, consider the performance implications of performing this here.

    // Task 9: set gl_Position to the object space position transformed to clip space
    vec4 viewSpacePos = viewMatrix * vec4(worldSpacePos, 1.0);
    gl_Position = projMatrix * viewSpacePos;
}
