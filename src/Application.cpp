//
// Created by Gjin on 10/26/25.
//

#include "Application.h"

#include "InterRegular.h"
#include "JetBrainsMono.h"
#include "ruleset.h"
#include "Viewport.h"
#include "simulation.h"
#include "themes.h"
#include "rlgl.h"

#define CHAR_BUFFER_SIZE 256



// TODO: ADD MORE CAMERA CONTROLS (WASD QE MOVEMENT, + RESET BUTTON)
// TODO: OPTIMIZE GRID WRAPPING
// TODO: POTENTIALLY OPTIMIZE UPDATE SIM STATE + COUNT NEIGHBORS METHODS
// TODO: DECOUPLE SIM SPEED FROM FPS
// TODO: IMPL INI READING FOR WINDOWS AND LINUX

class ModelMatrixApp final : public Application
{
    public:
        ModelMatrixApp() = default;
        ~ModelMatrixApp() override = default;

        // Create simulation members, initialize with default values
        Ruleset rs{"4/4,6/7", NeighborCountingRule::MOORE};
        std::pmr::vector<Color> activeColors = {DARKBLUE,BLUE,SKYBLUE,GREEN,YELLOW,ORANGE,RED};
        Simulation* simulation = new Simulation(100, rs, activeColors);

        // Fonts
        ImFont* interFont;
        ImFont* consoleFont;

        // Initialize app and state
        void startUp() override
        {
            // Apply theming, fonts, config flags, and update state
            SetTargetFPS(targetFpsValue);
            rlImGuiSetup(true);

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            ViewTexture = LoadRenderTexture(windowWidth, windowHeight);
            io.ConfigWindowsMoveFromTitleBarOnly = true;

            // Set INI path
#if defined(__APPLE__)
            io.IniFilename = "/Users/Shared/model-matrix.ini";
#elif defined(WIN32))
            io.IniFilename = "model-matrix.ini"; // TODO: test this change on windows
#endif
            // Start up viewport and simulation
            viewportWindow.Setup(*simulation, rs, activeColors);
            newColors.reserve(simulation->GetStateColors().size());
            simulation->RandomizeSimulationState(rngSparsity, cubeRadius, additiveFill, originPoint);

            themes::load_ue_dark();

            float dpi = ImGui::GetWindowDpiScale();

            // load fonts from font headers
            interFont = io.Fonts->AddFontFromMemoryCompressedTTF(
                InterRegular_compressed_data,
                InterRegular_compressed_size,
                interFontSize * dpi
            );

            consoleFont = io.Fonts->AddFontFromMemoryCompressedTTF(
                JetBrainsMono_compressed_data,
                JetBrainsMono_compressed_size,
                consoleFontSize * dpi
            );

            io.FontDefault = interFont;
        }

        // User interface code here
        void update() override
        {
            PushFontChanges();

            // set up dockspace
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

            // start update simulation thread
            std::thread simulateThread([this]()
            {
                this->simulation->UpdateSimulationState();
            });

            // update the viewport window
            viewportWindow.Update(*simulation, rs, activeColors);

            // join the simulation thread
            simulateThread.join();

            // Handle keybinds
            ProcessKeyboardInput();

            // draw the UI
            DrawMenuBar();
            DrawStatusWindow();
            DrawUsageGuideWindow();
            DrawAboutWindow();
            DrawRulesetSettingsWindow();
            DrawStateSettingsWindow();
            DrawDrawSettingsWindow();
            DrawViewportSettingsWindow();
            DrawSimulationSettingsWindow();
            DrawViewportWindow();


            PopFontChanges();
        }

    // Every var in private represents state
    private:
        // general flags
        bool isDarkMode = true;

        // Font size
        float interFontSize = 14.0f;
        float consoleFontSize = 15.0f;

        // Window show flags
        bool showViewport = true;
        bool showSimStatus = true;
        bool showUsageGuide = false;
        bool showAbout = false;

        // Settings window showflags
        bool showRulesetSettings = true;
        bool showStateSettings = true;
        bool showDrawSettings = true;
        bool showViewportSettings = true;
        bool showSimulationSettings = true;

        // Ruleset editor fields
        char rulesetField[CHAR_BUFFER_SIZE] = {"4/4,6/7"};
        int selectedCountingRule = 0;
        const char* availableCountingRules[2] = {"Moore", "Von Neumann"};
        std::vector<NeighborCountingRule> neighborCountingRules =
            {NeighborCountingRule::MOORE, NeighborCountingRule::VON_NEUMANN};
        std::string rsString;
        bool currentRulsetIsInvalid = false;
        // Color editor fields
        std::pmr::vector<Color> currentActiveColors;
        std::pmr::vector<Color> newColors;
        std::string colorStateLabelString;
        float stateColor[3];
        // Cell drawing fields
        float rngSparsity = 8.9;
        int cubeRadius = 30;
        bool additiveFill = false;
        int originPoint[3] = {0,0,0};
        // Viewport settings state value
        int resolution[2] = {1000,1000};
        float backgroundColors[3] = {0,0,0};
        float boundboxColors[3] = {1,1,1};
        bool fitToWindow = false;
        bool drawWireframe = false;
        // Simulation settings state values
        int targetFpsValue = 60;
        int simulationSize = 100;
        int advancementSpeed = 10;
        bool toggleGridWrapping = false;
        // Control panel state values
        Viewport viewportWindow;
        RenderTexture ViewTexture;
        bool isAdvancing = false;
        // General flags
        int windowInitState = 0;

        // UI FUNCTIONS
        void DrawMenuBar()
        {
            ImGui::BeginMainMenuBar();
            if (ImGui::MenuItem("Viewport"))
            {
                showViewport = !showViewport;
            }
            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("Ruleset Control"))
                {
                    showRulesetSettings = !showRulesetSettings;
                }
                if (ImGui::MenuItem("State Color Picker"))
                {
                    showStateSettings = !showStateSettings;
                }
                if (ImGui::MenuItem("Draw Cells"))
                {
                    showDrawSettings = !showDrawSettings;
                }
                if (ImGui::MenuItem("Viewport Settings"))
                {
                    showViewportSettings = !showViewportSettings;
                }
                if (ImGui::MenuItem("Simulation Settings"))
                {
                    showSimulationSettings = !showSimulationSettings;
                }
                ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Status"))
            {
                showSimStatus = !showSimStatus;
            }
            if (ImGui::MenuItem("Guide"))
            {
                showUsageGuide = !showUsageGuide;
            }
            if (ImGui::MenuItem("UI Theme"))
            {
                isDarkMode = !isDarkMode;

                if (isDarkMode)
                {
                    themes::load_ue_dark();
                } else
                {
                    ImGui::StyleColorsLight();
                }
            }
            if (ImGui::MenuItem("About"))
            {
                showAbout = !showAbout;
            }
            ImGui::EndMainMenuBar();
        }
        void DrawStatusWindow()
        {
            if (showSimStatus)
            {
                ImGui::Begin("Status", &showSimStatus);
                ImGui::PushFont(consoleFont);

                ImVec4 green;
                ImVec4 yellow;

                if (isDarkMode)
                {
                    green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                    yellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                } else
                {
                    green = ImVec4(0.0f, 0.5f, 0.0f, 1.0f);
                    yellow = ImVec4(0.75f, 0.75f, 0.0f, 1.0f);
                }

                if (isAdvancing)
                {
                    ImGui::TextColored(yellow, "ENGINE ADVANCING");
                    ImGui::Text("FPS: %d / %d", GetFPS(), advancementSpeed);
                } else {
                    if (simulation->IsSimulationRunning())
                    {
                        ImGui::TextColored(green, "ENGINE RUNNING");
                    } else
                    {
                        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "ENGINE IDLE");
                    }
                    ImGui::Text("FPS: %d / %d", GetFPS(), targetFpsValue);
                }
                ImGui::Text("OpenGL %i", rlGetVersion());
                ImGui::Text("Raylib %i.%i", RAYLIB_VERSION_MAJOR, RAYLIB_VERSION_MINOR);
                ImGui::Text("Threads: %i", simulation->getNumThreads());
                ImGui::Text("Font Size: %.0f", interFontSize);
                ImGui::PopFont();
                ImGui::End();
            }
        }
        void DrawUsageGuideWindow()
        {
            if (showUsageGuide)
            {
                // Row 5: Usage guide
                ImGui::Begin("Guide", &showUsageGuide);
                ImGui::Text("--- CONTROLS --- ");
                ImGui::Text("ENTER: toggle engine");
                ImGui::Text(" RIGHT: advance");
                ImGui::Text(" C: clear grid");
                ImGui::Text(" R/T: randomize grid");
                ImGui::Text(" +/-: adjust font size");
                ImGui::Text("\n");
                ImGui::Text("--- RULESET FORMATTING ---");
                ImGui::Text("<survivalConditions>/<birthConditions>/<numStates>");
                ImGui::Text("Example: '4/4,6/7'");
                ImGui::End();
            }
        }
        void DrawAboutWindow()
        {
            if (showAbout)
            {
                ImGui::Begin("About", &showAbout);
                ImGui::BeginChild("Contents", ImVec2(0, 0), true);
                ImGui::Text("MODEL-MATRIX");
                ImGui::Text("dev 0.9");
                ImGui::Text("A 3D Cellular Automata engine, "
                            "\nimplemented fully in C/C++"
                            "\n"
                            "\n"
                            "\nDeveloped by: Gjin Rexhaj");
                ImGui::EndChild();
                ImGui::End();
            }
        }

        void DrawViewportWindow()
        {
            if (showViewport)
            {
                viewportWindow.Show();

                if (windowInitState == 2)
                {
                    std::cout << "first frame! resizing viewport to fit." << std::endl;
                    resolution[0] = viewportWindow.GetWindowSize().at(0)+40;
                    resolution[1] = viewportWindow.GetWindowSize().at(1);
                    viewportWindow.UpdateViewportResolution(resolution[0], resolution[1]);
                    windowInitState = 3;
                } else if (windowInitState < 2)
                {
                    windowInitState++;
                }
            }
        }

        void DrawRulesetSettingsWindow()
        {
            if (showRulesetSettings)
            {
                ImGui::Begin("RULESET", &showRulesetSettings);
                // Row 1: Ruleset field
                // ImGui::BeginChild("rulesetContainer", ImVec2(0, 0), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("Controls", 2, ImGuiTableFlags_NoSavedSettings))
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Ruleset: ");
                    ImGui::SameLine();
                    if (ImGui::InputText("##RulesetInput", rulesetField, CHAR_BUFFER_SIZE))
                    {
                        currentRulsetIsInvalid = false;
                    };
                    ImGui::TableNextColumn();
                    ImGui::Text("Counting Rule: ");
                    ImGui::SameLine();
                    ImGui::Combo("##CountingruleInput", &selectedCountingRule, availableCountingRules, IM_ARRAYSIZE(availableCountingRules));
                    ImGui::TableNextColumn();
                    if (ImGui::Button("Apply Ruleset"))
                    {
                        try
                        {
                            rs = Ruleset(std::string(rulesetField), neighborCountingRules.at(selectedCountingRule));
                        } catch (std::exception &e)
                        {
                            currentRulsetIsInvalid = true;
                            std::cerr << e.what() << std::endl;
                            std::cout << "ruleset remains unchanged!"<< std::endl;
                        }
                        simulation->ChangeRuleset(std::string(rulesetField), neighborCountingRules.at(selectedCountingRule));
                        std::cout << "ruleset applied successfully!" << std::endl;
                    }
                    ImGui::TableNextColumn();
                    rsString = rs.GetRulesetAsString();
                    ImGui::Text("Active: ");
                    ImGui::SameLine();
                    if (currentRulsetIsInvalid)
                    {
                        ImGui::TextColored(ImVec4(255,0,0,255),"INVALID INPUT");
                    } else
                    {
                        ImGui::LabelText("##activeRS", rsString.c_str());
                    }
                    ImGui::EndTable();
                }
                // ImGui::EndChild();
                ImGui::End();
            }
        }

        void DrawStateSettingsWindow()
        {
            if (showStateSettings)
            {
                ImGui::Begin("STATE COLORS", &showStateSettings);
                currentActiveColors = simulation->GetStateColors();
                // ImGui::BeginChild("colorContainer", ImVec2(0, 0), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("ColorControls", 1, ImGuiTableFlags_NoSavedSettings))
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    int  i = 0;
                    for (auto color : simulation->GetStateColors())
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
                    simulation->ChangeStateColors(newColors);
                    newColors.clear();

                    ImGui::EndTable();
                }
                // ImGui::EndChild();
                ImGui::End();
            }
        }

        void DrawDrawSettingsWindow()
        {
            if (showDrawSettings)
            {
                ImGui::Begin("DRAW CELLS", &showDrawSettings);
                // ImGui::BeginChild("drawellsContainer", ImVec2(0, 0), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("Drawcellstable", 2, ImGuiTableFlags_NoSavedSettings))
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("RNG Sparsity ");
                    ImGui::SameLine();
                    ImGui::DragFloat("##dragFloatForSparity", &rngSparsity, 0.01f, 0.0f, 10.0f);
                    ImGui::Text("Cube Radius  ");
                    ImGui::SameLine();
                    ImGui::DragInt("##dragIntForRadius", &cubeRadius, 1, 0, ImGuiSliderFlags_ClampZeroRange);
                    ImGui::Text("Additive Fill");
                    ImGui::SameLine();
                    ImGui::Checkbox("##additiveFillToggle", &additiveFill);
                    ImGui::TableNextColumn();
                    ImGui::Text("PRESS 'R' FOR ONCE");
                    ImGui::Text( "HOLD 'T' FOR MULTIPLE");
                    ImGui::Text("PRESS 'C' TO CLEAR ALL");

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::EndTable();
                    ImGui::Text("Origin [x,y,z]: ");
                    ImGui::SameLine();
                    ImGui::DragInt3("##originDragger", originPoint);
                }
                // ImGui::EndChild();

                ImGui::End();
            }
        }

        void DrawViewportSettingsWindow()
        {
            if (showViewportSettings)
            {
                ImGui::Begin("VIEWPORT SETTINGS", &showViewportSettings);

                // ImGui::BeginChild("viewportSettingsContainer", ImVec2(0, 0), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("ViewportSettingsTable", 1, ImGuiTableFlags_NoSavedSettings)) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Res: ");
                    ImGui::SameLine();

                    if (ImGui::InputInt2("##resolutionInputInt2", resolution))
                    {
                        viewportWindow.UpdateViewportResolution(resolution[0], resolution[1]);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Fit to window"))
                    {
                        resolution[0] = viewportWindow.GetWindowSize().at(0)+40;
                        resolution[1] = viewportWindow.GetWindowSize().at(1);
                        viewportWindow.UpdateViewportResolution(resolution[0], resolution[1]);
                    };
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Background Color: ");
                    ImGui::SameLine();
                    if (ImGui::ColorEdit3("##ColorPickerLabelForVPBackground", backgroundColors))
                    {
                        Color newBackgroundColor = Color(backgroundColors[0]*255.0f, backgroundColors[1]*255.0f, backgroundColors[2]*255.0f, 255.0f);
                        viewportWindow.backgroundColor = newBackgroundColor;
                    }
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("BoundBox Color: ");
                    ImGui::SameLine();
                    if (ImGui::ColorEdit3("##ColorPickerLabelForVPBoundBox", boundboxColors))
                    {
                        Color newBoundboxColor = Color(boundboxColors[0]*255.0f, boundboxColors[1]*255.0f, boundboxColors[2]*255.0f, 255.0f);
                        simulation->boundingBoxColor = newBoundboxColor;
                    }
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Draw wireframe: ");
                    ImGui::SameLine();
                    if (ImGui::Checkbox("##wireframecheckbox", &drawWireframe))
                    {
                        simulation->toggleDrawWireframe();
                    }

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::EndTable();
                }
                // ImGui::EndChild();

                ImGui::End();
            }
        }

        void DrawSimulationSettingsWindow()
        {
            if (showSimulationSettings)
            {
                ImGui::Begin("SIMULATION SETTINGS", &showSimulationSettings);
                // ImGui::BeginChild("simulationSettingsContainer", ImVec2(0, 0), ImGuiChildFlags_Border);
                if (ImGui::BeginTable("SimulationSettingsTable", 1, ImGuiTableFlags_NoSavedSettings))
                {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Simulation speed (fps): ");
                    ImGui::SameLine();
                    if (ImGui::SliderInt("##fpsSlider", &targetFpsValue, 30, 120))
                    {
                        SetTargetFPS(targetFpsValue);
                    }
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Advancement speed (fps): ");
                    ImGui::SameLine();
                    if (ImGui::SliderInt("##advancementSlider", &advancementSpeed, 1, 120))
                    {
                        SetTargetFPS(targetFpsValue);
                    }
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::Text("Grid Size: ");
                    ImGui::SameLine();
                    ImGui::InputInt("##InputIntForSimSpan", &simulationSize, 5);
                    ImGui::SameLine();
                    if (ImGui::Button("Resize"))
                    {
                        simulation->ResizeSimulationSpan(simulationSize);
                    }
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Text("Grid wrapping: ");
                    ImGui::SameLine();
                    if (ImGui::Checkbox("##wraptoggle", &toggleGridWrapping))
                    {
                        simulation->toggleGridWrapping();
                    }

                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();

                    ImGui::EndTable();
                }
                // ImGui::EndChild();

                ImGui::End();
            }
        }

        void ProcessKeyboardInput()
        {
            if (simulation->IsSimulationRunning())
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    simulation->StopSimulation();
                }
            } else
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    simulation->StartSimulation();
                } else if (IsKeyDown(KEY_RIGHT))
                {
                    isAdvancing = true;
                    SetTargetFPS(advancementSpeed);
                    simulation->StartSimulation();
                    simulation->UpdateSimulationState();
                    simulation->StopSimulation();

                } else if (isAdvancing)
                {
                    SetTargetFPS(targetFpsValue);
                    isAdvancing = false;
                }

            }
            if (IsKeyPressed(KEY_R) || IsKeyDown(KEY_T))
            {
                simulation->RandomizeSimulationState(rngSparsity, cubeRadius, additiveFill, originPoint);
            } else if (IsKeyPressed(KEY_C))
            {
                simulation->ClearGrid();
            }
            // Handle font scaling
            if (IsKeyPressed(KEY_MINUS))
            {
                consoleFontSize--;
                interFontSize--;
            }

            if (IsKeyPressed(KEY_EQUAL))
            {
                consoleFontSize++;
                interFontSize++;
            }
        }

        void PushFontChanges()
        {
            ImGui::PushFont(consoleFont, consoleFontSize);
            ImGui::PushFont(interFont, interFontSize);
        }

        void PopFontChanges()
        {
            ImGui::PopFont();
            ImGui::PopFont();
        }
};



// Main method, instantiate and run app
int main()
{
    ModelMatrixApp app;
    app.run();

    return 0;
}