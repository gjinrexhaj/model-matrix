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
        int viewportWidth = 800;
        int viewportHeight = 800;

        // TODO: implement window position and size vars
        int windowX;
        int windowY;
        int windowWidth;
        int windowHeight;
        bool viewportHovered;


        void Setup()
        {
            ViewTexture = LoadRenderTexture(viewportWidth, viewportHeight);

            camera.fovy = 45;
            camera.up.y = 1;
            camera.position.y = 3;
            camera.position.z = -25;
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
                std::string camPosText = " Camera Position [" + std::to_string(camera.position.x) + ", " + std::to_string(camera.position.y) + ", " + std::to_string(camera.position.z) + "]";
                std::string isFocusedLabel = " Window focus: " + std::to_string(focused);
                std::string resolutionLabel = " Window resolution: [" + std::to_string(viewportWidth) + ", " + std::to_string(viewportHeight) + "]";
                std::string windowPositionLabel = " Window position: [" + std::to_string(windowX) + ", " + std::to_string(windowY) + "]";
                std::string windowSizeLabel = " Window size: [" + std::to_string(windowWidth) + ", " + std::to_string(windowHeight) + "]";
                std::string viewPortHoveredLabel = " ViewPort hovered: " + std::to_string(viewportHovered);

                ImGui::Text(camPosText.c_str());
                ImGui::Text(isFocusedLabel.c_str());
                ImGui::Text(resolutionLabel.c_str());
                ImGui::Text(windowPositionLabel.c_str());
                ImGui::Text(windowSizeLabel.c_str());
                ImGui::Text(viewPortHoveredLabel.c_str());

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

            // TODO: add camera controls;
            if (focused && viewportHovered)
            {
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    if (firstClick)
                    {
                        std::cout<<"first click"<<std::endl;
                        HideCursor();
                    } else
                    {
                        std::cout<<"mouse down"<< std::endl;
                        SetMousePosition(windowX + windowWidth/2, windowY + windowHeight/2);
                        HideCursor();
                    }

                    firstClick = false;
                } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                {
                    firstClick = true;
                    std::cout<<"mouse released"<< std::endl;
                    ShowCursor();
                }
            } else if (IsCursorHidden())
            {
                ShowCursor();
            }

            if (IsWindowResized())
            {
                UnloadRenderTexture(ViewTexture);
                ViewTexture = LoadRenderTexture(viewportWidth, viewportHeight);
            }

            BeginTextureMode(ViewTexture);
            ClearBackground(SKYBLUE);

            BeginMode3D(camera);

            // Begin draw model code
            DrawPlane(Vector3{ 0, 0, 0 }, Vector2{ 50, 50 }, BEIGE);
            float spacing = 4;
            int count = 5;

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
            // End draw model code

            EndMode3D();
            EndTextureMode();
        }

        void ShutDown()
        {
            UnloadRenderTexture(ViewTexture);;
        }


    private:
        RenderTexture ViewTexture = {};
        bool open = true;
        bool focused = false;
        Camera3D camera = {};
        bool firstClick = true;
};