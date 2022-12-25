#include "imgui.h"
#include "rlImGui.h"
#include "rlImGuiColors.h"
#include "pch.h"
#include "debug.h"
#include "grid.h"

enum PathFindingStatus {
    WaitingToStart,
    Processing,
    Sucessful,
    Failed,
};

const int gridSize = 24;
const int screenWidth = 1008;
const int screenHeight = 720;
const Color darkValues[4] = {{0, 0, 0, 255}, {14, 14, 14, 255}, {32, 32, 32, 255}, {64, 64, 64, 255}};
PathFindingStatus pathFindingStatus;
Color startColor = Color {113, 237, 110, 255};
Color blockColor = Color {214, 36, 17, 255};
Color closedPathColor = Color {53, 74, 178, 255};
Color openPathColor = Color {65, 133, 216, 255};
Color pathColor = Color {54, 39, 127, 255};
Grid grid;
bool windowHovered;
bool isPathFinding;
bool finishedPathFinding;
bool diagonalMoves;
bool findBestPath;
bool showProcess;
int gridColumns;
int iterationsPerFrame;
std::vector<Vector2> finalPath;
std::vector<int> squareAnims;
std::vector<Cell> openList;
std::vector<Cell> closedList;

void Initialize();
void InitStyle();
void UpdateFrame();
void DrawUI();
void DrawGrid();
void StartPathFinding();
void UpdatePathFinding(int maxIterations);

int main(int argc, char* argv[]) {
    InitWindow(screenWidth, screenHeight, "Pathfinding");
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    SetTargetFPS(60);
    rlImGuiSetup(true);
    ImGui::GetIO().IniFilename = NULL;
    
    Initialize();
    while (!WindowShouldClose()) {
        UpdateFrame();
    }

    CloseWindow();
    rlImGuiShutdown();

    return 0;
}

void Initialize() {
    squareAnims.clear();
    grid = Grid(screenWidth / gridSize, screenHeight / gridSize);
    squareAnims = std::vector<int>((signed) grid.values.size(), 0);
    windowHovered = true;
    isPathFinding = false;
    finishedPathFinding = false;
    diagonalMoves = true;
    iterationsPerFrame = 2;
    findBestPath = false;
    showProcess = true;
    pathFindingStatus = PathFindingStatus::WaitingToStart;
    InitStyle();
}

void InitStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 2.3f;
    style.FrameRounding = 1.3f;
    style.ScrollbarRounding = 0;

    style.Colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.90f, 0.90f);
    style.Colors[ImGuiCol_TextDisabled]          = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    style.Colors[ImGuiCol_WindowBg]              = ImVec4(0.09f, 0.09f, 0.15f, 1.00f);
    style.Colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 0.85f);
    style.Colors[ImGuiCol_Border]                = ImVec4(0.70f, 0.70f, 0.70f, 0.65f);
    style.Colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style.Colors[ImGuiCol_FrameBg]               = ImVec4(0.00f, 0.00f, 0.01f, 1.00f);
    style.Colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.90f, 0.80f, 0.80f, 0.40f);
    style.Colors[ImGuiCol_FrameBgActive]         = ImVec4(0.90f, 0.65f, 0.65f, 0.45f);
    style.Colors[ImGuiCol_TitleBg]               = ImVec4(0.00f, 0.00f, 0.00f, 0.83f);
    style.Colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.40f, 0.40f, 0.80f, 0.20f);
    style.Colors[ImGuiCol_TitleBgActive]         = ImVec4(0.00f, 0.00f, 0.00f, 0.87f);
    style.Colors[ImGuiCol_MenuBarBg]             = ImVec4(0.01f, 0.01f, 0.02f, 0.80f);
    style.Colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.20f, 0.25f, 0.30f, 0.60f);
    style.Colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.55f, 0.53f, 0.55f, 0.51f);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
    style.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.56f, 0.56f, 0.56f, 0.91f);
    style.Colors[ImGuiCol_CheckMark]             = ImVec4(0.90f, 0.90f, 0.90f, 0.83f);
    style.Colors[ImGuiCol_SliderGrab]            = ImVec4(0.70f, 0.70f, 0.70f, 0.62f);
    style.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.30f, 0.30f, 0.30f, 0.84f);
    style.Colors[ImGuiCol_Button]                = ImVec4(0.48f, 0.72f, 0.89f, 0.49f);
    style.Colors[ImGuiCol_ButtonHovered]         = ImVec4(0.50f, 0.69f, 0.99f, 0.68f);
    style.Colors[ImGuiCol_ButtonActive]          = ImVec4(0.80f, 0.50f, 0.50f, 1.00f);
    style.Colors[ImGuiCol_Header]                = ImVec4(0.30f, 0.69f, 1.00f, 0.53f);
    style.Colors[ImGuiCol_HeaderHovered]         = ImVec4(0.44f, 0.61f, 0.86f, 1.00f);
    style.Colors[ImGuiCol_HeaderActive]          = ImVec4(0.38f, 0.62f, 0.83f, 1.00f);
    style.Colors[ImGuiCol_ResizeGrip]            = ImVec4(1.00f, 1.00f, 1.00f, 0.85f);
    style.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(1.00f, 1.00f, 1.00f, 0.60f);
    style.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);
    style.Colors[ImGuiCol_PlotLines]             = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    style.Colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogram]         = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    style.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.00f, 0.00f, 1.00f, 0.35f);

}

void UpdateFrame() {
    BeginDrawing();
        ClearBackground(darkValues[2]);
        DrawGrid();
        DrawUI();

        if (IsKeyPressed(KEY_R))
            grid.Clear();

    EndDrawing();

    if (isPathFinding)
        UpdatePathFinding(iterationsPerFrame);
}

void DrawUI() {
    rlImGuiBegin();

    bool open = true;
    ImGui::Begin("Path Finding", &open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Spacing();
    ImGui::Text("Current status:");
    ImGui::SameLine();
    switch (pathFindingStatus)
    {
    case PathFindingStatus::WaitingToStart:
        ImGui::TextColored(rlImGuiColors::Convert(GRAY), "Waiting to start");
        break;
    case PathFindingStatus::Processing:
        ImGui::TextColored(rlImGuiColors::Convert(openPathColor), "Proccessing");
        break;
    case PathFindingStatus::Sucessful:
        ImGui::TextColored(rlImGuiColors::Convert(startColor), "Sucessful");
        break;
    case PathFindingStatus::Failed:
        ImGui::TextColored(rlImGuiColors::Convert(blockColor), "Failed");
        break;
    
    default:
        break;
    }

    ImGui::Spacing();
    ImGui::DragInt("Iterations per frame", &iterationsPerFrame);

    ImGui::Spacing();
    ImGui::Checkbox("Diagonal moves allowed", &diagonalMoves);

    ImGui::Spacing();
    ImGui::Checkbox("Show path finding processus", &showProcess);

    ImGui::Spacing();
    ImGui::Checkbox("Find fastest path (slower)", &findBestPath);

    ImGui::Spacing();
    if (ImGui::Button("Reset Board"))
        grid.Clear();
        
    ImGui::Spacing();
    if (ImGui::Button("Clear path")) {
        finalPath.clear();
        pathFindingStatus = PathFindingStatus::WaitingToStart;
    }

    ImGui::Spacing();
    if (ImGui::Button("Start Path Finding"))
        StartPathFinding();
    
    if (isPathFinding) {
        ImGui::Indent();
        ImGui::Spacing();
        if (ImGui::Button("Step Path Finding"))
            UpdatePathFinding(1);

        ImGui::Spacing();
        if (ImGui::Button("Step Path Finding")) {
            isPathFinding = false;
            pathFindingStatus = PathFindingStatus::Failed;
        }           
    }

    windowHovered = ImGui::IsWindowHovered() || (ImGui::IsWindowFocused() && !IsMouseButtonDown(MOUSE_BUTTON_RIGHT));

    ImGui::End();
    rlImGuiEnd();
}

void DrawGrid() {
    Vector2 mp = GetMousePosition();
    DrawCircleGradient(mp.x, mp.y, 150, darkValues[2], ColorAlpha(darkValues[3], 0));

    for (int x = 0; x < grid.width; x++) {
        for (int y = 0; y < grid.height; y++) {
            Rectangle rect = {(float) x * gridSize, (float) y * gridSize, gridSize, gridSize};
            DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, darkValues[1]);

            if ((x == 0 && y == 0) || (x == grid.width - 1 && y == grid.height - 1)) {
                DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, startColor);
                continue;
            }

            if (CheckCollisionPointRec(mp, rect)) {
                if (!windowHovered) {
                    bool pressedLeft = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
                    bool pressedRight = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);
                    if (pressedLeft && !pressedRight) {
                        grid.SetAt(x, y, 1);
                    } else if (pressedRight && !pressedLeft) {
                        grid.SetAt(x, y, 0);
                    }
                }
            }

            int index = grid.IndexAt(x, y);
            if (grid.GetAt(x, y) == 1) {
                if (squareAnims[index] < 10)
                    squareAnims[index]++;
            } else {
                if (squareAnims[index] > 0) {
                    squareAnims[index] -= 2;
                    if (squareAnims[index] < 0) {
                        squareAnims[index] = 0;
                    }

                }
            }

            if (squareAnims[index]) {
                int sizeOffset = sizeOffset = 10 - squareAnims[index];
                rect.x += sizeOffset;
                rect.y += sizeOffset;
                rect.width -= sizeOffset * 2;
                rect.height -= sizeOffset * 2;

                DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, blockColor);
            } else {
                for (auto &vec : finalPath)
                    if (vec.x == x && vec.y == y)
                        DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, pathColor);
                
                if (isPathFinding && showProcess) {
                    for (auto &cell : openList)
                        if (cell.pos.x == x && cell.pos.y == y)
                            DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, openPathColor);
                    
                    for (auto &cell : closedList)
                        if (cell.pos.x == x && cell.pos.y == y)
                            DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, closedPathColor);
                }
            }
        }
    }
}

void StartPathFinding() {
    isPathFinding = true;
    finishedPathFinding = false;
    finalPath.clear();
    openList.clear();
    closedList.clear();
    openList.push_back(Cell {0, 0, 0, {0, 0}});
    pathFindingStatus = PathFindingStatus::Processing;
}

void UpdatePathFinding(int maxIterations) {
    Vector2 endingPos = {(float) grid.width - 1, (float) grid.height - 1};

    for (int iteration = 0; iteration < maxIterations; iteration++) {
        if (!openList.size()) {
            isPathFinding = false;
            finishedPathFinding = false;
            pathFindingStatus = PathFindingStatus::Failed;
            return;
        }

        int bestCellIndex = 0;
        for (int index = 0; index < (signed) openList.size(); index++) {
            if (openList[index].f < openList[bestCellIndex].f)
                bestCellIndex = index;
        }

        Cell currentCell = openList[bestCellIndex];
        openList.erase(openList.begin() + bestCellIndex);
        closedList.push_back(currentCell);
        int parentCellIndex = (signed) closedList.size() - 1;

        for (int x = -1; x < 2; x++) {
            for (int y = -1; y < 2; y++) {
                if (x == 0 && y == 0) continue;

                if (!diagonalMoves)
                    if (x != 0 && y != 0)
                        continue;

                Vector2 childPos = {currentCell.pos.x + x, currentCell.pos.y + y};
                
                // If we have found the ending cell
                if (childPos.x == endingPos.x && childPos.y == endingPos.y) {
                    pathFindingStatus = PathFindingStatus::Sucessful;
                    isPathFinding = false;
                    finishedPathFinding = true;
                    finalPath.push_back(childPos);

                    // Construct the final path
                    int currentIndex = parentCellIndex;
                    while (true) {
                        finalPath.insert(finalPath.begin(), closedList[currentIndex].pos);
                        if (currentIndex == 0)
                            break;
                        currentIndex = closedList[currentIndex].parentIndex;
                    }

                    return;
                }

                // Is position out of bounds
                if (!grid.IsOnBoard(childPos.x, childPos.y)) continue;

                // Is position blocked
                if (grid.GetAt(childPos.x, childPos.y) == 1) continue;

                // Is position already in the closed list
                bool foundInClosed = false;
                for (Cell &closedCell : closedList) {
                    if (closedCell.pos.x == childPos.x && closedCell.pos.y == childPos.y) {
                        foundInClosed = true;
                        break;
                    }
                }

                if (foundInClosed)
                    continue;

                // Calculate f value
                int g;
                if (findBestPath)
                    g = currentCell.g + 10;
                else
                    g = currentCell.g + ((x != 0 && y != 0) ? 14 : 10);

                int xDistance = std::abs(childPos.x - endingPos.x);
                int yDistance = std::abs(childPos.y - endingPos.y);
                int h = std::sqrt((double) std::pow(xDistance, 2) + std::pow(yDistance, 2)) * 10;
                int f = g + h;

                bool foundInOpen = false;
                for (Cell &openCell : openList) {
                    if (openCell.pos.x == childPos.x && openCell.pos.y == childPos.y) {
                        if (g > openCell.g) {
                            openCell.g = g;
                            openCell.f = f;
                            openCell.parentIndex = parentCellIndex;
                        }

                        foundInOpen = true;
                        break;
                    }
                }

                if (foundInOpen)
                    continue;

                openList.push_back(Cell {g, f, parentCellIndex, childPos});
            }
        }
    }
}
