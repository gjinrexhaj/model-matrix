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
                ImGui::TableNextColumn();
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
                ImGui::BeginTable("Controls", 1, ImGuiTableFlags_NoSavedSettings);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                int  i = 0;
                for (auto color : currentActiveColors)
                {
                    std::string labelString = "State " + std::to_string(i+1) + " color: ";
                    float cols[3] = {(float)color.r, (float)color.g, (float)color.b};
                    cols[0] /= 255.0f;
                    cols[1] /= 255.0f;
                    cols[2] /= 255.0f;
                    ImGui::PushID(i);
                    ImGui::Text(labelString.c_str());
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
                // Color adjustment logic
                simulation.ChangeStateColors(newColors);
                newColors.clear();

                ImGui::EndTable();
                ImGui::EndChild();

                /*
                ImGui::Text("Ruleset: ");
                ImGui::SameLine();
                ImGui::InputText("##RulesetInput", rulesetField, CHAR_BUFFER_SIZE);
                ImGui::SameLine();
                ImGui::Combo("##CountingruleInput", &selectedCountingRule, availableCountingRules, IM_ARRAYSIZE(availableCountingRules));
                ImGui::SameLine();
                if (ImGui::Button("Apply Ruleset")) { std::cout << "apply ruleset!" << std::endl; }

                ImGui::LabelText("Ruleset: ", rulesetField);
                ImGui::LabelText("counting rule: ", availableCountingRules[selectedCountingRule]);
                */
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
        std::pmr::vector<Color> currentActiveColors;
        std::pmr::vector<Color> newColors;

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