//
// Created by Gjin on 10/26/25.
//

#include "tester.h"

#include <iostream>
#include <ostream>

#include "raylib.h"
#include "raymath.h"


#include "imgui.h"
#include "rlImGui.h"


int main()
{
    int screenWidth = 1280;
    int screenHeight = 800;

    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT | FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib-Extras [ImGui] example - simple ImGui Demo");
    SetTargetFPS(144);
    rlImGuiSetup(true);


    // Main game loop
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(DARKGRAY);

        // start ImGui Conent
        rlImGuiBegin();

        // show ImGui Content
        bool open = true;

        if (ImGui::Begin("Test Window", &open))
        {
            ImGui::TextUnformatted(ICON_FA_JEDI);

        }
        ImGui::End();

        // end ImGui Content
        rlImGuiEnd();
        EndDrawing();

    }


    rlImGuiShutdown();
    CloseWindow();        // Close window and OpenGL context

    return 0;
}
