//
// Created by Gjin on 10/26/25.
//

#pragma once

#include <iostream>

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "Viewport.h"

class Viewport
{
    public:
        float orbitRadius = 10.0f;
        Vector2 viewportResolution = {1600,1600};
        int radius = 10;
        int cameraFov;
        float panSensitivity;
        int scrollSpeed;

        void Setup()
        {
            ViewTexture = LoadRenderTexture(viewportResolution.x, viewportResolution.y);

            camera.fovy = 45;
            camera.projection = CAMERA_PERSPECTIVE;
            camera.position = {0.0f, 10.0f, 10.0f};
            camera.target = {0.0f, 0.0f, 0.0f};
            camera.up = {0.0f, 1.0f, 0.0f};

            angleX = 0;
            angleY = 0;
            panSensitivity = 0.005f;
            scrollSpeed = 4;

            UpdateCameraState();
        }

        void Show()
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

            if (ImGui::Begin("3D Viewport", &open, ImGuiWindowFlags_NoScrollbar))
            {
                // Update window data
                ImVec2 windowCoords = ImGui::GetWindowPos();
                windowX = windowCoords.x;
                windowY = windowCoords.y;

                ImVec2 windowSize = ImGui::GetWindowSize();
                windowWidth = windowSize.x;
                windowHeight = windowSize.y;


                focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

                // draw the viewport
                rlImGuiImageRenderTextureFit(&ViewTexture, true);
                viewportHovered = ImGui::IsItemHovered();

                // Debug data labels
                ImGui::BeginChild("Debug");
                std::string camPosText = " Camera Position [" + std::to_string(camera.position.x) + ", " + std::to_string(camera.position.y) + ", " + std::to_string(camera.position.z) + "]";
                std::string isFocusedLabel = " Window focus: " + std::to_string(focused);
                std::string resolutionLabel = " Viewport resolution: [" + std::to_string(viewportResolution.x) + ", " + std::to_string(viewportResolution.y) + "]";
                std::string windowPositionLabel = " Window position: [" + std::to_string(windowX) + ", " + std::to_string(windowY) + "]";
                std::string windowSizeLabel = " Window size: [" + std::to_string(windowWidth) + ", " + std::to_string(windowHeight) + "]";
                std::string viewPortHoveredLabel = " ViewPort hovered: " + std::to_string(viewportHovered);

                ImGui::Text(camPosText.c_str());
                ImGui::Text(isFocusedLabel.c_str());
                ImGui::Text(resolutionLabel.c_str());
                ImGui::Text(windowPositionLabel.c_str());
                ImGui::Text(windowSizeLabel.c_str());
                ImGui::Text(viewPortHoveredLabel.c_str());
                ImGui::EndChild();

            }

            ImGui::PopStyleVar();
            ImGui::End();
        }

        void Update()
        {

            if (!open)
            {
                return;
            }

            // Control camera on viewport grab focus, hide mouse + return to center when let go
            if (focused && viewportHovered)
            {
                radius -= static_cast<int>(GetMouseWheelMove() * scrollSpeed);

                if (radius < 1)
                {
                    radius = 1;
                }

                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    if (firstClick)
                    {
                        HideCursor();
                    } else
                    {
                        HideCursor();
                        // Handle camera controls
                        HandleCameraControls();
                        SetMousePosition(windowX + windowWidth/2, windowY + windowHeight/2);
                    }
                    firstClick = false;
                } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                {
                    firstClick = true;
                    ShowCursor();
                }

                UpdateCameraState();

            } else if (IsCursorHidden())
            {
                ShowCursor();
            }

            if (IsWindowResized())
            {
                UnloadRenderTexture(ViewTexture);
                ViewTexture = LoadRenderTexture(viewportResolution.x, viewportResolution.y);
            }

            BeginTextureMode(ViewTexture);
            ClearBackground(SKYBLUE);
            BeginMode3D(camera);

            // Begin draw model code
            RenderToViewport();
            // End draw model code

            EndMode3D();
            EndTextureMode();
        }

        void ShutDown()
        {
            UnloadRenderTexture(ViewTexture);;
        }


        void HandleCameraControls()
        {
            camera.position += Vector3(0, 1, 0);

            // Only control camera if it isn't at the origin
            Vector2 mouseDelta = GetMouseDelta();
            angleX += mouseDelta.x * panSensitivity; // Adjust sensitivity
            angleY += mouseDelta.y * panSensitivity; // Adjust sensitivity

            // Clamp vertical angle to prevent flipping
            if (angleY > PI / 2.0f - 0.1f) angleY = PI / 2.0f - 0.1f;
            if (angleY < -PI / 2.0f + 0.1f) angleY = -PI / 2.0f + 0.1f;

        }

        void UpdateCameraState()
        {
            camera.position.x = radius * cosf(angleX) * cosf(angleY);
            camera.position.y = radius * sinf(angleY);
            camera.position.z = radius * sinf(angleX) * cosf(angleY);
        }

        void RenderToViewport()
        {
            // TODO: right now all this method does is create a demo scene. Integrate with simulation drawing once
            //  it's been implemented
            int cubeSize = 1;

            DrawCube(Vector3{0,0,0}, cubeSize, cubeSize, cubeSize, WHITE);
            DrawCube(Vector3{1,0,0}, cubeSize, cubeSize, cubeSize, GRAY);
            DrawCube(Vector3{0,1,0}, cubeSize, cubeSize, cubeSize, BLUE);
            DrawCube(Vector3{-1,0,0}, cubeSize, cubeSize, cubeSize, GREEN);
            DrawPlane(Vector3{ 0, 0, 0 }, Vector2{ 50, 50 }, BEIGE);
            float spacing = 5;
            int count = 5;

            DrawText("hello lol", 0,0, 12, WHITE);

            for (float x = -count * spacing; x <= count * spacing; x += spacing)
            {
                for (float z = -count * spacing; z <= count * spacing; z += spacing)
                {
                    Vector3 pos = { x, 0.5f, z };

                    Vector3 min = { x - 0.5f,0,z - 0.5f };
                    Vector3 max = { x + 0.5f,1,z + 0.5f };

                    DrawCube(Vector3{ x, 1.5f, z }, 1, 1, 1, GREEN);
                    DrawCube(Vector3{ x, 0.5f, z }, 0.25f, 1, 0.25f, BROWN);
                }
            }
        }


    private:
        RenderTexture ViewTexture = {};
        bool open = true;
        bool focused = false;
        Camera3D camera = {};
        bool firstClick = true;
        float angleX = 0;
        float angleY = 0;
        int windowX;
        int windowY;
        int windowWidth;
        int windowHeight;
        bool viewportHovered;
};