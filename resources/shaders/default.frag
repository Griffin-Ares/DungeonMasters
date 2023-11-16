#version 330 core

// Task 5: declare "in" variables for the world-space position and normal,
//         received post-interpolation from the vertex shader
in vec3 worldSpacePos;
in vec3 normal;

// Task 10: declare an out vec4 for your output color
out vec3 fragColor;

// Task 12: declare relevant uniform(s) here, for ambient lighting
uniform float k_a;

// Task 13: declare relevant uniform(s) here, for diffuse lighting
uniform float k_d;

uniform vec4 lightDirs[8];
uniform vec4 lightColors[8];
uniform float lightCount;

// Task 14: declare relevant uniform(s) here, for specular lighting
uniform float k_s;
uniform float shininess;
uniform vec4 camPos;

uniform vec4 matAmbient;
uniform vec4 matDiffuse;
uniform vec4 matSpecular;


void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized

    // Task 10: set your output color to white (i.e. vec4(1.0)). Make sure you get a white circle!
    //fragColor = vec3(1.0);

    // Task 11: set your output color to the absolute value of your world-space normals,
    //          to make sure your normals are correct.
    //fragColor = vec4(abs(normal), 1.0);


    // Task 12: add ambient component to output color
    fragColor = vec3(k_a) * matAmbient.xyz;

    // Task 13: add diffuse component to output color
    //vec3 lightDir = normalize(lightPos.xyz - worldSpacePos);
    for (int i = 0; i < lightCount; ++i) {
        vec3 lightDir = normalize(-lightDirs[i].xyz);
        vec3 norm = normalize(normal);
        float lightDot = max(dot(norm, lightDir), 0.0);

        fragColor += lightColors[i].xyz * vec3(k_d * lightDot) * matDiffuse.xyz;

        // Task 14: add specular component to output color
        vec3 reflectedLight = reflect(-lightDir, norm);
        vec3 viewDir = vec3(normalize(camPos - vec4(worldSpacePos, 1.0)));

        float specIntensity = shininess > 0 ? pow(max(dot(viewDir, reflectedLight), 0.0), shininess) : 1;
        //float specIntensity = pow(max(dot(viewDir, reflectedLight), 0.0), shininess);
        fragColor += lightColors[i].xyz * vec3(k_s * specIntensity) * matSpecular.xyz;
    }
}
