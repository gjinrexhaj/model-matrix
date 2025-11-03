//
// Created by Gjin on 10/26/25.
//

#include "Application.h"
#include  "Viewport.h"
#include "simulation.h"

class ModelMatrixApp final : public Application
{
    public:
        ModelMatrixApp() = default;
        ~ModelMatrixApp() override = default;

        // Create simulation members, initialize with default values
        Ruleset ruleset{2,2,2,NeighborCountingRule::MOORE};
        std::pmr::vector<Color> colors = {WHITE};
        Simulation simulation {7, ruleset, colors};

        // Initialize app and state
        void startUp() override
        {
            // Apply theming, fonts, config flags, and update state
            SetTargetFPS(144);
            rlImGuiSetup(true);

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            ViewTexture = LoadRenderTexture(windowWidth, windowHeight);
            io.ConfigWindowsMoveFromTitleBarOnly = true;

            // Initialize default simulation, ruleset, and color state
            /*
            ruleset = Ruleset(4,4,2, NeighborCountingRule::MOORE);
            colors.push_back(Color(1,1,1));
            simulation = Simulation(10, ruleset, colors);
            */

            // Start up viewport
            viewportWindow.Setup(simulation, ruleset, colors);
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
            viewportWindow.Update(simulation, ruleset, colors);
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