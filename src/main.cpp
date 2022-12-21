#include "pch.h"
#include "debug.h"
#include "imgui.h"
#include "rlImGui.h"

const int screenWidth = 1020;
const int screenHeight = 700;
const Color darkValues[4] = {{0, 0, 0, 255}, {14, 14, 14, 255}, {32, 32, 32, 255}, {64, 64, 64, 255}};

Vector2 scroll;
int gridSize;
bool isDragging;
Vector2 dragginPos;
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
    gridSize = 24;
    windowLoc = {0, 0, 0, 0};
    isDragging = false;
    scroll = {0, 0};

}

void UpdateFrame() {
    Vector2 mp = GetMousePosition();

    BeginDrawing();
        ClearBackground(darkValues[1]);
        DrawGrid();
        DrawUI();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !CheckCollisionPointRec(GetMousePosition(), windowLoc)) {
            if (isDragging) {
                scroll.x -= mp.x - dragginPos.x;
                scroll.y -= mp.y - dragginPos.y;
            }

            isDragging = true;
            dragginPos = GetMousePosition();
        } else {
            isDragging = false;
        }
        
    EndDrawing();
}

void DrawUI() {
    rlImGuiBegin();

    bool open = true;
    ImGui::Begin("Path Finding", &open);

    ImVec2 pos = ImGui::GetWindowPos();
    ImVec2 size = ImGui::GetWindowSize();
    windowLoc = {pos.x, pos.y, size.x, size.y};

    ImGui::End();
    rlImGuiEnd();
}

void DrawGrid() {
    for (int x = gridSize - ((int) scroll.x % gridSize); x < screenWidth; x+=gridSize) {
        DrawLine(x, 0, x, screenHeight, darkValues[2]);
    }
    for (int y = gridSize - ((int) scroll.y % gridSize); y < screenHeight; y+=gridSize) {
        DrawLine(0, y, screenWidth, y, darkValues[2]);
    }
}
