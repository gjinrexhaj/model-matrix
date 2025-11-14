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



// TODO: OPTIMIZE PROGRAM
// TODO: GET RID OF OLD RULSET CLASS
// TODO: handle error checking for when number of eleemnts in newStateColors != number
//  of states as defined in active ruleset
// TODO: impl theming
// TODO: impl fps counter+graph on stats window

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
            newColors.reserve(simulation.GetStateColors().size());
            simulation.RandomizeSimulationState(rngSparsity, cubeRadius, additiveFill);

            themes::load_ue();

        }

        // User interface code here
        void update() override
        {
            // set up dockspace
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            // Update simulation
            simulation.UpdateSimulationState();

            ImGui::ShowDemoWindow();

            // menu bar for opening windows
            ImGui::BeginMainMenuBar();
            if (ImGui::MenuItem("Viewport"))
            {
                showViewport = !showViewport;
            }
            if (ImGui::MenuItem("Control Panel"))
            {
                showControlPanel = !showControlPanel;
            }
            if (ImGui::MenuItem("Status"))
            {
                showSimStatus = !showSimStatus;
            }
            if (ImGui::MenuItem("Guide"))
            {
                showUsageGuide = !showUsageGuide;
            }
            if (ImGui::MenuItem("About"))
            {
                showAbout = !showAbout;
            }
            ImGui::EndMainMenuBar();

            // 3d viewport window
            viewportWindow.Update(simulation, rulesetNew, activeColors);
            if (showViewport)
            {
                viewportWindow.Show();
                // check for inputs
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
            }
            // Live info window
            if (showSimStatus)
            {
                ImGui::Begin("Status", &showSimStatus);
                if (simulation.IsSimulationRunning())
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "ENGINE RUNNING");
                } else
                {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ENGINE IDLE");
                }
                ImGui::Text("FPS: %d", GetFPS());
                ImGui::End();
            }

            // Usage guide
            if (showUsageGuide)
            {
                // Row 5: Usage guide
                ImGui::Begin("Guide", &showUsageGuide);
                ImGui::Text("CONTROLS: ");
                ImGui::Text("ENTERKEY: PAUSE/PLAY SIMULATION (TOGGLE)");
                ImGui::Text("RG_ARROW: ADVANCE (HOLD)");
                ImGui::Text("LF_ARROW: REGRESS (HOLD)");
                ImGui::Text("\n");
                ImGui::Text("RULESET FORMATTING:");
                ImGui::Text("<survivalConditions>/<birthConditions>/<numStates>");
                ImGui::Text("Example: '4/4,6/7'");
                ImGui::End();
            }

            // About window
            if (showAbout)
            {
                ImGui::Begin("About", &showAbout);
                ImGui::BeginChild("Contents", ImVec2(0, 0), true);
                ImGui::Text("MODEL-MATRIX");
                ImGui::Text("dev 0.7");
                ImGui::Text("A 3D Cellular Automata engine, "
                            "\nimplemented fully in C/C++"
                            "\n"
                            "\n"
                            "\nDeveloped by: Gjin Rexhaj");
                ImGui::EndChild();
                ImGui::End();
            }

            // control panel window
            if (showControlPanel)
            {
                ImGui::Begin("Control Panel", &showControlPanel);

                // Row 1: Ruleset field
                ImGui::Text("RULESET");
                ImGui::BeginChild("rulesetContainer", ImVec2(0, 70), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("Controls", 2, ImGuiTableFlags_NoSavedSettings))
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Ruleset: ");
                    ImGui::SameLine();
                    ImGui::InputText("##RulesetInput", rulesetField, CHAR_BUFFER_SIZE);
                    ImGui::TableNextColumn();
                    ImGui::Text("Counting Rule: ");
                    ImGui::SameLine();
                    ImGui::Combo("##CountingruleInput", &selectedCountingRule, availableCountingRules, IM_ARRAYSIZE(availableCountingRules));
                    //ImGui::TableNextRow();
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
                }
                ImGui::EndChild();

                // Row 2: State color picker
                currentActiveColors = simulation.GetStateColors();
                ImGui::Text("STATE COLORS");
                ImGui::BeginChild("colorContainer", ImVec2(0, 180), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("ColorControls", 1, ImGuiTableFlags_NoSavedSettings))
                {
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
                }
                ImGui::EndChild();


                // Row 3: Grid drawing
                ImGui::Text("DRAW CELLS");
                ImGui::BeginChild("drawellsContainer", ImVec2(0, 90), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("Drawcellstable", 2, ImGuiTableFlags_NoSavedSettings))
                {
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
                }
                ImGui::EndChild();


                // Row 4: Viewport settings (resolution, bounding box color, background color - lighting (if we get there), multithreading toggle, grid size toggle
                ImGui::Text("VIEWPORT SETTINGS");
                ImGui::BeginChild("viewportSettingsContainer", ImVec2(0, 140), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("ViewportSettingsTable", 1, ImGuiTableFlags_NoSavedSettings)) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Res: ");
                    ImGui::SameLine();
                    if (!fitToWindow)
                    {
                        ImGui::InputInt2("##resolutionInputInt2", resolution);
                    } else
                    {
                        ImGui::BeginDisabled();
                        resolution[0] = viewportWindow.GetWindowSize().at(0)+40;
                        resolution[1] = viewportWindow.GetWindowSize().at(1);
                        ImGui::InputInt2("##resolutionInputInt2disabled", resolution);
                        ImGui::EndDisabled();
                    }
                    ImGui::SameLine();
                    ImGui::Text("Auto: ");
                    ImGui::SameLine();
                    ImGui::Checkbox("##autofitViewportToWindow", &fitToWindow);
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
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if (ImGui::Button("Apply Viewport Changes"))
                    {
                        viewportWindow.UpdateViewportResolution(resolution[0], resolution[1]);
                        simulation.ResizeSimulationSpan(simulationSize);
                        Color newBackgroundColor = Color(backgroundColors[0]*255.0f, backgroundColors[1]*255.0f, backgroundColors[2]*255.0f, 255.0f);
                        Color newBoundboxColor = Color(boundboxColors[0]*255.0f, boundboxColors[1]*255.0f, boundboxColors[2]*255.0f, 255.0f);
                        viewportWindow.backgroundColor = newBackgroundColor;
                        simulation.boundingBoxColor = newBoundboxColor;
                    }
                    ImGui::EndTable();
                }
                ImGui::EndChild();
                ImGui::End();
            }

        }

    // Every var in private represents state
    private:
        // Window show flags
        bool showViewport = true;
        bool showSimStatus = true;
        bool showControlPanel = true;
        bool showUsageGuide = true;
        bool showAbout = false;
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
        bool fitToWindow = false;


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