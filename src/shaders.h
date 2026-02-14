#pragma once

namespace shaders
{
    // vertex shader used in batch rendering
    inline const char* INSTANCING_VERTEX = R"glsl(
        #version 330
        layout (location = 0) in vec3 vertexPosition;
        layout (location = 2) in vec3 vertexNormal; // Location 2 is standard for normals
        layout (location = 4) in mat4 instanceTransform;

        uniform mat4 mvp;

        out vec3 fragNormal;

        void main() {
            fragNormal = normalize(mat3(instanceTransform) * vertexNormal);

            gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
        }
    )glsl";

    // fragment shader used in batch rendering
    inline const char* INSTANCING_FRAGMENT = R"glsl(
        #version 330
        in vec3 fragNormal;
        uniform vec4 colDiffuse;
        out vec4 finalColor;

        void main() {
            vec3 lightDir = normalize(vec3(0.4, 1.0, 0.4));
            float diff = max(dot(fragNormal, lightDir), 0.0);

            float emission = 0.8;
            float lightIntensity = clamp(diff + emission, 0.0, 1.5);

            finalColor = vec4(colDiffuse.rgb * lightIntensity, colDiffuse.a);
        }
    )glsl";
}