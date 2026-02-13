#pragma once

namespace shaders
{
    // vertex shader used in batch rendering
    inline const char* INSTANCING_VERTEX = R"glsl(
        #version 330
        in vec3 vertexPosition;
        in vec3 vertexNormal;
        in mat4 instanceTransform;

        uniform mat4 mvp;

        void main() {
            gl_Position = mvp * instanceTransform * vec4(vertexPosition, 1.0);
        }
    )glsl";

    // fragment shader used in batch rendering
    inline const char* INSTANCING_FRAGMENT = R"glsl(
        #version 330
        uniform vec4 colDiffuse;
        out vec4 finalColor;

        void main() {
            finalColor = colDiffuse;
        }
    )glsl";
}