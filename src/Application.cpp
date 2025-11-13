//
// Created by Gjin on 10/26/25.
//

#include "Application.h"

#include "imgui_internal.h"
#include "ruleset_new.h"
#include  "Viewport.h"
#include "simulation.h"
#include <algorithm>

#define CHAR_BUFFER_SIZE 256


// TODO: FINISH GUI
// TODO: FIX IMGUI SEGFAULT BUG
// TODO: IMPL START + STOP SIMULATION
// TODO: IMPL SIMSPACE RESIZING
// TODO: GET RID OF OLD RULSET CLASS

class ModelMatrixApp final : public Application
{
    public:
        ModelMatrixApp() = default;
        ~ModelMatrixApp() override = default;

        // Create simulation members, initialize with default values
        // DEAD DOES NOT COUNT AS A STATE
        RulesetNew rulesetNew{"4/4,6/7", NeighborCountingRule::MOORE};
        std::pmr::vector<Color> activeColors = {DARKPURPLE,VIOLET,BLUE,SKYBLUE,GREEN,GOLD,YELLOW};
        Simulation simulation {70, rulesetNew, activeColors};

        // Initialize app and state
        void startUp() override
        {
            // Apply theming, fonts, config flags, and update state
            SetTargetFPS(60);
            rlImGuiSetup(true);

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            ViewTexture = LoadRenderTexture(windowWidth, windowHeight);
            io.ConfigWindowsMoveFromTitleBarOnly = true;

            // Start up viewport and simulation
            viewportWindow.Setup(simulation, rulesetNew, activeColors);
            simulation.StartSimulation();

            newColors.reserve(simulation.GetStateColors().size());
        }

        // User interface code here
        void update() override
        {
            // set up dockspace
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            // Update simulation
            simulation.UpdateSimulationState();
            // 3d viewport window
            viewportWindow.Update(simulation, rulesetNew, activeColors);
            if (showViewport)
            {
                viewportWindow.Show();
            }
            // control panel window
            if (showControlPanel)
            {
                ImGui::Begin("Control Panel", &showControlPanel);

                // Row 1: Ruleset field
                ImGui::Text("RULESET");
                ImGui::BeginChild("rulesetContainer", ImVec2(0, 70), ImGuiChildFlags_Border);
                ImGui::BeginTable("Controls", 2, ImGuiTableFlags_NoSavedSettings);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Ruleset: ");
                ImGui::SameLine();
                ImGui::InputText("##RulesetInput", rulesetField, CHAR_BUFFER_SIZE);
                ImGui::TableNextColumn();
                ImGui::Text("Counting Rule: ");
                ImGui::SameLine();
                ImGui::Combo("##CountingruleInput", &selectedCountingRule, availableCountingRules, IM_ARRAYSIZE(availableCountingRules));
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Button("Apply Ruleset"))
                {
                    rulesetNew = RulesetNew(std::string(rulesetField), neighborCountingRules.at(selectedCountingRule));
                    simulation.ChangeRuleset(std::string(rulesetField), neighborCountingRules.at(selectedCountingRule));
                }
                ImGui::TableNextColumn();
                std::string rsString = rulesetNew.GetRulesetAsString();
                ImGui::Text("Active: ");
                ImGui::SameLine();
                ImGui::LabelText("##activeRS", rsString.c_str());
                ImGui::EndTable();
                ImGui::EndChild();

                // Row 2: State color picker
                currentActiveColors = simulation.GetStateColors();
                ImGui::Text("STATE COLORS");
                ImGui::BeginChild("colorContainer", ImVec2(0, 180), ImGuiChildFlags_Border);
                ImGui::BeginTable("ColorControls", 1, ImGuiTableFlags_NoSavedSettings);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                int  i = 0;
                for (auto color : simulation.GetStateColors())
                {
                    colorStateLabelString = "State " + std::to_string(i+1) + " color: ";
                    float cols[3] = {(float)color.r, (float)color.g, (float)color.b};
                    cols[0] /= 255.0f;
                    cols[1] /= 255.0f;
                    cols[2] /= 255.0f;
                    ImGui::PushID(i);
                    ImGui::Text(colorStateLabelString.c_str());
                    ImGui::SameLine();
                    ImGui::ColorEdit3("##xx", cols, ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float);
                    i++;
                    cols[0] *= 255.0f;
                    cols[1] *= 255.0f;
                    cols[2] *= 255.0f;
                    Color custom = Color(cols[0], cols[1], cols[2], 255.0f);
                    newColors.push_back(custom);
                    ImGui::PopID();
                }
                simulation.ChangeStateColors(newColors);
                newColors.clear();

                ImGui::EndTable();
                ImGui::EndChild();

                // Row 3: Grid drawing
                ImGui::Text("DRAW CELLS");
                ImGui::BeginChild("drawellsContainer", ImVec2(0, 90), ImGuiChildFlags_Border);
                ImGui::BeginTable("Drawcellstable", 2, ImGuiTableFlags_NoSavedSettings);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("RNG Sparsity ");
                ImGui::SameLine();
                ImGui::DragFloat("##dragFloatForSparity", &rngSparsity, 0.01f, 0.0f, 10.0f);
                ImGui::Text("Cube Radius  ");
                ImGui::SameLine();
                ImGui::DragInt("##dragIntForRadius", &cubeRadius, 1, 0, 100);
                ImGui::Text("Additive Fill");
                ImGui::SameLine();
                ImGui::Checkbox("##additiveFillToggle", &additiveFill);
                ImGui::TableNextColumn();
                ImGui::Text("PRESS 'R' FOR ONCE");
                ImGui::Text( "HOLD 'T' FOR MULTIPLE");
                ImGui::Text("PRESS 'C' TO CLEAR ALL");
                if (IsKeyPressed(KEY_R) || IsKeyDown(KEY_T))
                {
                    simulation.RandomizeSimulationState(rngSparsity, cubeRadius, additiveFill);
                } else if (IsKeyPressed(KEY_C))
                {
                    simulation.ClearGrid();
                }
                ImGui::EndTable();
                ImGui::EndChild();

                // Row 4: Viewport settings (resolution, bounding box color, background color - lighting (if we get there), multithreading toggle, grid size toggle
                ImGui::Text("VIEWPORT SETTINGS");
                ImGui::BeginChild("viewportSettingsContainer", ImVec2(0, 120), ImGuiChildFlags_Border);
                ImGui::EndChild();

                // Row 5: Usage guide

                ImGui::End();
            }

        }

    // Every var in private represents state
    private:
        // Window show flags
        bool showViewport = true;
        bool showControlPanel = true;
        // Ruleset editor fields
        char rulesetField[CHAR_BUFFER_SIZE] = {};
        int selectedCountingRule = 0;
        const char* availableCountingRules[2] = {"Moore", "Von Neumann"};
        std::vector<NeighborCountingRule> neighborCountingRules =
            {NeighborCountingRule::MOORE, NeighborCountingRule::VON_NEUMANN};
        // Color editor fields
        std::pmr::vector<Color> currentActiveColors;
        std::pmr::vector<Color> newColors;
        std::string colorStateLabelString;
        // Cell drawing fields
        float rngSparsity = 8.9;
        int cubeRadius = 5;
        bool additiveFill = false;


        // Control panel state values
        Viewport viewportWindow;
        RenderTexture ViewTexture;
};



// Main method, instantiate and run app
int main()
{
    ModelMatrixApp app;
    app.run();

    return 0;
}