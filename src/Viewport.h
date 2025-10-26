//
// Created by Gjin on 10/26/25.
//

#pragma once

#include "raylib.h"
#include "imgui.h"
#include "rlImGui.h"
#include "Viewport.h"

class Viewport
{
    public:
        void Setup()
        {
            ViewTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());

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
                focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);
                // draw the view
                rlImGuiImageRenderTextureFit(&ViewTexture, true);
                std::string camPosText = "Camera Position [" + std::to_string(camera.position.x) + ", " + std::to_string(camera.position.y) + ", " + std::to_string(camera.position.z) + "]";
                std::string isFocusedLabel = "Window focus: " + std::to_string(focused);

                ImGui::Text(camPosText.c_str());
                ImGui::Text(isFocusedLabel.c_str());
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

            // TODO: add camera controls; check if focused

            if (IsWindowResized())
            {
                UnloadRenderTexture(ViewTexture);
                ViewTexture = LoadRenderTexture(GetScreenWidth(), GetScreenHeight());
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
};