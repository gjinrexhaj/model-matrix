//
// Created by Gjin on 10/26/25.
//

#include "Application.h"

#include "ruleset_new.h"
#include  "Viewport.h"
#include "simulation.h"

class ModelMatrixApp final : public Application
{
    public:
        ModelMatrixApp() = default;
        ~ModelMatrixApp() override = default;

        // Create simulation members, initialize with default values
        // DEAD DOES NOT COUNT AS A STATE
        RulesetNew rulesetNew{"4/4,6/7", NeighborCountingRule::MOORE};
        std::pmr::vector<Color> colors = {DARKPURPLE,VIOLET,BLUE,SKYBLUE,GREEN,GOLD,YELLOW};
        Simulation simulation {100, rulesetNew, colors};

        // Initialize app and state
        void startUp() override
        {
            rulesetNew.PrintRulesetAsString();
            // Apply theming, fonts, config flags, and update state
            SetTargetFPS(60);
            rlImGuiSetup(true);

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            ViewTexture = LoadRenderTexture(windowWidth, windowHeight);
            io.ConfigWindowsMoveFromTitleBarOnly = true;

            // Start up viewport and simulation
            viewportWindow.Setup(simulation, rulesetNew, colors);

            simulation.StartSimulation();
        }

        // User interface code here
        void update() override
        {
            // set up dockspace
            ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
            // example ui
            ImGui::ShowDemoWindow();

            // Update simulation
            simulation.UpdateSimulationState();
            // 3d viewport window
            viewportWindow.Update(simulation, rulesetNew, colors);
            if (showViewport)
            {
                viewportWindow.Show();
            }

        }

    // Every var in private represents state
    private:
        int someNumber = 5;
        RenderTexture ViewTexture;
        bool showViewport = true;
        Viewport viewportWindow;
};



// Main method, instantiate and run app
int main()
{
    ModelMatrixApp app;
    app.run();

    return 0;
}