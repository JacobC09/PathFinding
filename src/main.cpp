#include "raylib.h"
#include "raymath.h"
#include "imgui.h"
#include "rlImGui.h"

const int screenWidth = 1020;
const int screenHeight = 700;
const Color darkValues[4] = {{0, 0, 0, 255}, {14, 14, 14, 255}, {32, 32, 32, 255}, {64, 64, 64, 255}};

int gridSize;
bool isDragging;
Vector2 dragStart;
Rectangle windowLoc;

void Initialize();
void UpdateFrame();
void DrawUI();
void DrawGrid();

int main(int argc, char* argv[]) {
    InitWindow(screenWidth, screenHeight, "Pathfinding");
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTargetFPS(60);
    rlImGuiSetup(true);
    
    Initialize();
    while (!WindowShouldClose()) {
        UpdateFrame();
    }

    CloseWindow();
    rlImGuiShutdown();

    return 0;
}

void Initialize() {
    windowLoc = {0, 0, 0, 0};
    gridSize = screenWidth / 32;
    isDragging = false;
}

void UpdateFrame() {
    BeginDrawing();
        ClearBackground(darkValues[1]);
        DrawGrid();
        DrawUI();

        if (CheckCollisionPointRec(GetMousePosition(), windowLoc)) {

        }
        
    EndDrawing();
}

void DrawUI() {
    rlImGuiBegin();

    bool open = true;
    ImGui::Begin("Path Finding", &open);

    ImGui::SliderInt("Grid Size", &gridSize, screenWidth / 64 + 1, screenWidth / 16 + 1);

    if (ImGui::Button("Start"));

    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    windowLoc = {pos.x, pos.y, size.x, size.y};

    ImGui::End();
    rlImGuiEnd();
}

void DrawGrid() {
    for (int index = 0; index <= gridSize; index++) {
        int pos = screenWidth * (float) index / gridSize;

        if (pos < screenWidth)
            DrawLine(pos, 0, pos, screenHeight, darkValues[2]);
        if (pos < screenHeight)
            DrawLine(0, pos, screenWidth, pos, darkValues[2]);
    }
}
