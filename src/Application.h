//
// Created by Gjin on 10/26/25.
// Application class: framework for building applications with ImGui
//

#pragma once

#include <raylib.h>
#include <raymath.h>
#include <imgui.h>
#include <rlImGui.h>
#include <string>

class Application
{
    public:

        Application()
        {
            // Initialize ImGui + rlImGui
            IMGUI_CHECKVERSION();
            SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
            InitWindow(windowWidth, windowHeight, windowTitle.c_str());
        }

        virtual ~Application()
        {
            rlImGuiShutdown();
            CloseWindow();
        }

        void run()
        {
            // Call subclasses' specified state initialization method
            startUp();
            // Main loop
            while (!WindowShouldClose())
            {
                BeginDrawing();
                ClearBackground(DARKGRAY);
                // start ImGui Content
                rlImGuiBegin();

                // Call subclasses' specified update method
                update();

                // end ImGui Content
                rlImGuiEnd();
                EndDrawing();
            }
        }
        virtual void update()
        {

        }

        virtual void startUp()
        {

        }


        // Global variables
        int windowWidth = 1280, windowHeight = 720;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        std::string windowTitle = "model-matrix";


};
