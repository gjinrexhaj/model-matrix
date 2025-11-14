//
// Created by Gjin on 10/26/25.
//

#include "Application.h"

#include "imgui_internal.h"
#include "ruleset_new.h"
#include  "Viewport.h"
#include "simulation.h"
#include "themes.h"

#define CHAR_BUFFER_SIZE 256


// TODO: FIX IMGUI SEGFAULT BUG
// TODO: FINISH GUI
// TODO: IMPL SIMSPACE RESIZING - impl accurate resize method in grid that preserves ordering
// TODO: OPTIMIZE PROGRAM
// TODO: GET RID OF OLD RULSET CLASS
// TODO: handle error checking for when number of eleemnts in newStateColors != number
//  of states as defined in active ruleset
// TODO: impl theming

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
            //themes::load_excellency(); // TODO: test theming after segfault bug is fixed
            SetTargetFPS(60);
            rlImGuiSetup(true);

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            ViewTexture = LoadRenderTexture(windowWidth, windowHeight);
            io.ConfigWindowsMoveFromTitleBarOnly = true;

            // Start up viewport and simulation
            viewportWindow.Setup(simulation, rulesetNew, activeColors);
            newColors.reserve(simulation.GetStateColors().size());
            simulation.RandomizeSimulationState(rngSparsity, cubeRadius, additiveFill);
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
            // Live info window
            if (showSimInfo)
            {
                ImGui::Begin("Simulation Info", &showSimInfo);
                if (simulation.IsSimulationRunning())
                {
                    ImGui::Text("ENGINE RUNNING");
                } else
                {
                    ImGui::Text("ENGINE IDLE");
                }
                ImGui::Text("FPS: %d", GetFPS());
                ImGui::End();
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
                    stateColor[0] = (float)color.r/255.0f;
                    stateColor[1] = (float)color.g/255.0f;
                    stateColor[2] = (float)color.b/255.0f;
                    ImGui::PushID(i);
                    ImGui::Text(colorStateLabelString.c_str());
                    ImGui::SameLine();
                    ImGui::ColorEdit3("##xx", stateColor, ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float);
                    i++;
                    stateColor[0] *= 255.0f;
                    stateColor[1] *= 255.0f;
                    stateColor[2] *= 255.0f;
                    Color custom = Color(stateColor[0], stateColor[1], stateColor[2], 255.0f);
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
                ImGui::BeginChild("viewportSettingsContainer", ImVec2(0, 140), ImGuiChildFlags_Border);
                ImGui::BeginTable("Drawcellstable", 1, ImGuiTableFlags_NoSavedSettings);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Resolution: ");
                ImGui::SameLine();
                ImGui::InputInt2("##resolutionInputInt2", resolution);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Grid Size: ");
                ImGui::SameLine();
                ImGui::InputInt("##InputIntForSimSpan", &simulationSize);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Background Color: ");
                ImGui::SameLine();
                ImGui::ColorEdit3("##ColorPickerLabelForVPBackground", backgroundColors);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("BoundBox Color: ");
                ImGui::SameLine();
                ImGui::ColorEdit3("##ColorPickerLabelForVPBoundBox", boundboxColors);
                if (ImGui::Button("Apply Viewport Changes"))
                {
                    // TODO: impl methods in viewport and sim to change these values
                    viewportWindow.viewportResolution = Vector2(resolution[0], resolution[1]);
                    simulation.ResizeSimulationSpan(simulationSize);
                    Color newBackgroundColor = Color(backgroundColors[0]*255.0f, backgroundColors[1]*255.0f, backgroundColors[2]*255.0f, 255.0f);
                    Color newBoundboxColor = Color(boundboxColors[0]*255.0f, boundboxColors[1]*255.0f, boundboxColors[2]*255.0f, 255.0f);
                    viewportWindow.backgroundColor = newBackgroundColor;
                    simulation.boundingBoxColor = newBoundboxColor;
                }
                ImGui::EndTable();
                ImGui::EndChild();

                // Row 5: Usage guide
                ImGui::Text("USAGE GUIDE");
                ImGui::BeginChild("usageGuideSettingsContainer", ImVec2(0, 80), ImGuiChildFlags_Border);
                ImGui::Text("ENTERKEY: PAUSE/PLAY SIMULATION (TOGGLE)");
                ImGui::Text("RG_ARROW: FORWARDS (HOLD)");
                ImGui::Text("LF_ARROW: BACKWARD (HOLD)");
                if (simulation.IsSimulationRunning())
                {
                    if (IsKeyPressed(KEY_ENTER))
                    {
                        simulation.StopSimulation();
                    }
                } else
                {
                    if (IsKeyPressed(KEY_ENTER))
                    {
                        simulation.StartSimulation();
                    } else if (IsKeyDown(KEY_RIGHT))
                    {
                        simulation.StartSimulation();
                        simulation.UpdateSimulationState();
                        simulation.StopSimulation();
                    }
                }
                ImGui::EndChild();
                ImGui::End();
            }

        }

    // Every var in private represents state
    private:
        // Window show flags
        bool showViewport = true;
        bool showSimInfo = true;
        bool showControlPanel = true;
        // Ruleset editor fields
        char rulesetField[CHAR_BUFFER_SIZE] = {"4/4,6/7"};
        int selectedCountingRule = 0;
        const char* availableCountingRules[2] = {"Moore", "Von Neumann"};
        std::vector<NeighborCountingRule> neighborCountingRules =
            {NeighborCountingRule::MOORE, NeighborCountingRule::VON_NEUMANN};
        // Color editor fields
        std::pmr::vector<Color> currentActiveColors;
        std::pmr::vector<Color> newColors;
        std::string colorStateLabelString;
        float stateColor[3];
        // Cell drawing fields
        float rngSparsity = 8.9;
        int cubeRadius = 30;
        bool additiveFill = false;
        // Viewport settings state value
        int resolution[2] = {1000,1000};
        int simulationSize = 70;
        float backgroundColors[3] = {0,0,0};
        float boundboxColors[3] = {1,1,1};


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