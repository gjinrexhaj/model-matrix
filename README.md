# Idea
goal: build a customizable 3D cellular automata engine,
which uses string manipulation to programmatically
generate rulesets based on user-specified rules

Example rule
```
4/4/5/M
```
State 1 cell survives if it has 4 neighbors.
A Cell is born in an empty location if it has 4 neighbors.
5 Specifies that the cells can have a minimum state of 5.
M means use Moores neighbor counting system.

# Tech Stack

- Lang: C/C++
- Gui: Dear Imgui
- Graphics: RayLib

Use rlImGui to bridge the two.

# Architecture
Class files for simulation; maybe similar to 2d CA. Main cpp file.

# UX

window contains entire app space, tabs for viewport, as well as settings, also camera settings

- viewport: contains camera that shows 3d view of grid
- settings: tweak color, simulation speed, ruleset, all changes update in realtime
- utils: play, pause, etc.
- camera settings: auto-rotate, othrographic v perspective, fov, etc.
