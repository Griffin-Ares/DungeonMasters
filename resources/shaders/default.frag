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

struct Light {
    int type;      // 1: directional || 0: point || 3: spot
    vec4 pos;
    vec4 dir;
    vec4 color;
    vec3 att;
    float angle; // spot only
    float penumbra; // spot only
};

uniform Light lights[8];
uniform int lightCount;

// Task 14: declare relevant uniform(s) here, for specular lighting
uniform float k_s;
uniform float shininess;
uniform vec4 camPos;

uniform vec4 matAmbient;
uniform vec4 matDiffuse;
uniform vec4 matSpecular;


void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized

    fragColor = vec3(k_a) * matAmbient.xyz;

    //vec3 lightDir = normalize(lightPos.xyz - worldSpacePos);
    for (int i = 0; i < lightCount; ++i) {
        vec3 lightPos;
        float fatt;
        float distance;

        switch (lights[i].type) {
            // point
            case(0):
                distance = length(lights[i].pos.xyz - worldSpacePos);
                lightPos = normalize(lights[i].pos.xyz - worldSpacePos);
                fatt = min(1.0f, 1.0f/(lights[i].att[0] + distance * lights[i].att[1] + pow(distance, 2) * lights[i].att[2]));
                break;

            // directional
            case(1):
                lightPos = normalize(-lights[i].dir.xyz);
                fatt = 1.0f;
                break;

            // spot
            case(2):
            distance = length(lights[i].pos.xyz - worldSpacePos);
                lightPos = normalize(lights[i].pos.xyz - worldSpacePos);
                fatt = min(1.0f, 1.0f/(lights[i].att[0] + distance * lights[i].att[1] + pow(distance, 2) * lights[i].att[2]));

                float cosineAngle = abs(dot(lightPos, normalize(vec3(lights[i].dir))));

                if (cosineAngle < cos(lights[i].angle)) {
                    continue;
                }

                float angleAtt = 1.f;
                float innerConeCos = cos(lights[i].angle - lights[i].penumbra);
                if (cosineAngle < innerConeCos) {
                    float t = (cosineAngle - innerConeCos) / (cos(lights[i].angle) - innerConeCos);
                    angleAtt = 1.f - (-2.f * pow(t, 3) + 3.f * pow(t, 2));
                }

                fatt *= abs(angleAtt);
                break;
        }


        vec3 norm = normalize(normal);
        float lightDot = max(dot(norm, lightPos), 0.0);

        // Task 14: add specular component to output color
        vec3 reflectedLight = reflect(-lightPos, norm);
        vec3 viewDir = vec3(normalize(camPos - vec4(worldSpacePos, 1.0f)));

        float specIntensity = shininess > 0 ? pow(max(dot(viewDir, reflectedLight), 0.0f), shininess) : 1.f;

        fragColor += fatt * lights[i].color.xyz * ((vec3(k_d * lightDot) * matDiffuse.xyz) + (vec3(k_s * specIntensity) * matSpecular.xyz));
    }
}
