#version 330 core

// Task 16: Create a UV coordinate in variable
in vec2 texCoord;

// Task 8: Add a sampler2D uniform

// variable naming not my strong suit
uniform sampler2D texture2D;

// Task 29: Add a bool on whether or not to filter the texture
uniform bool doPostProcessPixel;
uniform bool doPostProcessKernel;

out vec4 fragColor;

void main()
{
    if (doPostProcessKernel) {
        float kernel[9] = float[](-1, -1, -1, -1, 17, -1, -1, -1, -1);
        vec2 texOffset = 1.0 / textureSize(texture2D, 0);
        vec3 result = vec3(0.0);

        for (int x = -1; x <= 1; x++) {
            for (int y = -1; y <= 1; y++) {
                vec2 offset = vec2(texOffset.x * x, texOffset.y * y);
                result += texture(texture2D, texCoord + offset).rgb * 1.f/9.f * kernel[(y + 1) * 3 + (x + 1)];
            }
        }
        fragColor = vec4(result, 1.0);
    } else {
        fragColor = texture(texture2D, texCoord);
    }

    // Task 33: Invert fragColor's r, g, and b color channels if your bool is true
    if (doPostProcessPixel) {
        fragColor.rgb = 1.0 - fragColor.rgb;
    }
}
