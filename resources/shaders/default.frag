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

// NORMAL MAPPING :D
uniform sampler2D brickMap;
uniform sampler2D floorMap;
uniform int isTextured; // true if == 1, false otherwise
mat3 posX; mat3 negX; mat3 posZ; mat3 negZ; mat3 posY; // TBNs

/* naming is a little confusing, so just to clarify: these two functions return colors that
  will represent normals after being mapped from color range [0, 1] to normal range [-1, 1]
  the actual colors of the bricks and the floor are hardcoded in applyNormalMapping() */
vec3 sampleBrickColor(float x, float y) {
    int intX = int(round(x));
    int intY = int(round(y));
    float textureU = float(intX % 5) / 5.0; // TOCHECK: that 5 is the height of the walls
    float textureV = float(intY % 5) / 5.0;
    return vec3(texture(brickMap, vec2(textureU, textureV)));
};

vec3 sampleFloorColor() {
    int intX = int(round(worldSpacePos.x));
    int intZ = int(round(worldSpacePos.z));
    float textureU = float(intX % 5) / 5.0;
    float textureV = float(intZ % 5) / 5.0;
    return vec3(texture(floorMap, vec2(textureU, textureV)));
};

vec3 applyNormalMapping() {
    vec3 newNormal;
    vec3 sampledNormal;
    // if normal is pointing up or down, use concrete floor map
    if (dot(normal, vec3(0, 1, 0)) > 0.999 || dot(normal, vec3(0, -1, 0)) > 0.999) {
        fragColor = vec3(103, 99, 100); // brownish grey for floor
        sampledNormal = sampleFloorColor();
        newNormal = 2 * sampledNormal - vec3(1.f); // maps from color range to normal range
        newNormal = posY * newNormal; // bring normal from normal map space into world space
    // otherwise, use brick wall map
    } else {
        fragColor = vec3(137, 120, 102); // reddish brown for bricks
        vec3 sampledNormal;
        // sample normal from map, convert from [0, 1] to [-1, 1], and transform into world space
        if (dot(normal, vec3(1, 0, 0)) > 0.999) { // wall facing positive x direction
            sampledNormal = sampleBrickColor(worldSpacePos.z, worldSpacePos.y);
            newNormal = 2 * sampledNormal - vec3(1.f);
            newNormal = posX * newNormal;
        } else if (dot(normal, vec3(-1, 0, 0)) > 0.999) { // wall facing negative x direction
            sampledNormal = sampleBrickColor(-worldSpacePos.z, worldSpacePos.y);
            newNormal = 2 * sampledNormal - vec3(1.f);
            newNormal = negX * newNormal;
        } else if (dot(normal, vec3(0, 0, 1)) > 0.999) { // wall facing positive z direction
            sampledNormal = sampleBrickColor(-worldSpacePos.x, worldSpacePos.y);
            newNormal = 2 * sampledNormal - vec3(1.f);
            newNormal = posZ * newNormal;
        } else if (dot(normal, vec3(0, 0, -1)) > 0.999) { // wall facing negative z direction
            sampledNormal = sampleBrickColor(worldSpacePos.x, worldSpacePos.y);
            newNormal = 2 * sampledNormal - vec3(1.f);
            newNormal = negZ * newNormal;
        } else {
            newNormal = normal;
        }
    }
    return newNormal;
}


void main() {
    // Remember that you need to renormalize vectors here if you want them to be normalized

    /* comment out below stuff when ready to turn on normal mapping */
    fragColor = vec3(k_a) * matAmbient.xyz;
    vec3 norm = normalize(normal);
    /* uncomment below stuff when ready to turn on normal mapping */
//        vec3 norm;
//        if (isTextured == 1) {
//            norm = applyNormalMapping();
//            // fragColor is adjusted within applyNormalMapping()
//        } else {
//            norm = normalize(normal);
//            fragColor = matAmbient.xyz;
//        }
//    fragColor = vec3(k_a) * fragColor;

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

        float lightDot = max(dot(norm, lightPos), 0.0);

        // Task 14: add specular component to output color
        vec3 reflectedLight = reflect(-lightPos, norm);
        vec3 viewDir = vec3(normalize(camPos - vec4(worldSpacePos, 1.0f)));

        float specIntensity = shininess > 0 ? pow(max(dot(viewDir, reflectedLight), 0.0f), shininess) : 1.f;

        fragColor += fatt * lights[i].color.xyz * ((vec3(k_d * lightDot) * matDiffuse.xyz) + (vec3(k_s * specIntensity) * matSpecular.xyz));
    }
}
