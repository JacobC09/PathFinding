#include "imgui.h"
#include "rlImGui.h"
#include "pch.h"
#include "debug.h"
#include "grid.h"

const int gridSize = 24;
const int screenWidth = 1008;
const int screenHeight = 720;
const Color darkValues[4] = {{0, 0, 0, 255}, {14, 14, 14, 255}, {32, 32, 32, 255}, {64, 64, 64, 255}};
Color green = Color {73, 242, 39, 255};
Color red = Color {219, 24, 35, 255};
Color blue = Color {27, 112, 247, 255};
Color cyan = Color {2, 188, 240, 255};
Color darkCyan = Color {6, 120, 158, 255};
Grid grid;
bool windowHovered;
bool isPathFinding;
bool finishedPathFinding;
bool cornerMovesAllowed;
int gridColumns;
int iterationsPerFrame;
std::vector<Vector2> finalPath;
std::vector<int> squareAnims;
std::vector<Cell> openList;
std::vector<Cell> closedList;

void Initialize();
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
    cornerMovesAllowed = true;
    iterationsPerFrame = 4;
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
    ImGui::DragInt("Iterations per frame", &iterationsPerFrame, 1, 0);

    ImGui::Spacing();
    ImGui::Checkbox("Corner moves allowed", &cornerMovesAllowed);

    ImGui::Spacing();
    if (ImGui::Button("Reset Board"))
        grid.Clear();
        
    ImGui::Spacing();
    if (ImGui::Button("Clear path"))
        finalPath.clear();

    ImGui::Spacing();
    if (ImGui::Button("Start Path Finding"))
        StartPathFinding();
    
    if (isPathFinding) {
        ImGui::Indent();
        ImGui::Spacing();
        if (ImGui::Button("Step Path Finding"))
            UpdatePathFinding(1);
    }

    windowHovered = ImGui::IsWindowHovered() || ImGui::IsWindowFocused();

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
                DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, green);
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

                DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, red);
            } else {
                for (auto &vec : finalPath)
                    if (vec.x == x && vec.y == y)
                        DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, blue);
                
                if (isPathFinding) {
                    for (auto &cell : openList)
                        if (cell.pos.x == x && cell.pos.y == y)
                            DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, cyan);
                        
                    for (auto &cell : closedList)
                        if (cell.pos.x == x && cell.pos.y == y)
                            DrawRectangle(rect.x, rect.y, rect.width - 1, rect.height - 1, darkCyan);
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

    // Add the starting position
    openList.push_back(Cell {0, 0, 0, {0, 0}});
}

void UpdatePathFinding(int maxIterations) {
    Vector2 endingPos = {(float) grid.width - 1, (float) grid.height - 1};

    for (int iteration = 0; iteration < maxIterations; iteration++) {
        if (!openList.size()) {
            isPathFinding = false;
            finishedPathFinding = false;
            print("The path is blocked :(");
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

                if (!cornerMovesAllowed)
                    if (x != 0 && y != 0)
                        continue;

                Vector2 childPos = {currentCell.pos.x + x, currentCell.pos.y + y};
                
                // If we have found the ending cell
                if (childPos.x == endingPos.x && childPos.y == endingPos.y) {
                    print("Mission successful folks! :D");
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
                int g = currentCell.g + (x != 0 && y != 0) ? 14 : 10;
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

    //         # Child is already in the open list
    //         found = False
    //         for openCell in openList:
    //             if openCell.position == childPos and g > openCell.g:
    //                 found = True
    //                 continue
            
    //         if found:
    //             continue

    //         # Add the child to the open list
    //         openList.append(Cell(g, f, childPos, len(closedList) - 1))
        

// void UpdatePathFinding(int maxIterations) {
//     Vector2 endingPos = {(float) grid.width - 1, (float) grid.height - 1};

//     for (int iterations; iterations < maxIterations; iterations++) {
//         // If there is not where else to go
//         if (!openList.size()) {
//             isPathFinding = false;
//             finishedPathFinding = false;
//             print("The path is blocked :(");
//             return;
//         }

//         // Find the node with the lowest f cost
//         int bestCellIndex = 0;
//         for (int index = 0; index < (signed) openList.size(); index++)
//             if (openList[index].f < openList[bestCellIndex].f)
//                 bestCellIndex = index;
        
//         Cell currentCell = openList[bestCellIndex];
        
//         // Remove current node from the open list
//         openList.erase(openList.begin() + bestCellIndex);
        
//         // Add current node to closed list
//         closedList.push_back(currentCell);
//         int currentCellIndex = (signed) closedList.size() - 1;

//         for (int x = -1; x < 2; x++) {
//             for (int y = 0; y < 2; y++) {
//                 if (x == 0 && y == 0) continue;
                
//                 Vector2 childPos = {currentCell.pos.x + x, currentCell.pos.y + y};
                
//                 // If we have found the cell
//                 if (childPos.x == endingPos.x && childPos.y == endingPos.y) {
//                     print("Mission successful folks! :D");
//                     isPathFinding = false;
//                     finishedPathFinding = true;
//                     finalPath.push_back(childPos);

//                     // Construct the final path
//                     int currentIndex = (signed) closedList.size() - 1;
//                     while (true) {
//                         finalPath.insert(finalPath.begin(), closedList[currentIndex].pos);

//                         if (currentIndex == 0) break;

//                         currentIndex = closedList[currentIndex].originIndex;
//                     }

//                     return;
//                 }

//                 // If child position is invalid
//                 if (!grid.IsOnBoard(childPos.x, childPos.y) || grid.GetAt(childPos.x, childPos.y) == 1) continue;

//                 // If child position is not already in the closed list
//                 bool inClosedList = false;
//                 for (Cell &closedCell : closedList) {
//                     if (closedCell.pos.x == childPos.x && closedCell.pos.y == childPos.y) {
//                         inClosedList = true;
//                         break;
//                     }
//                 }
//                 if (inClosedList)
//                     continue;
            
//                 int g;
//                 if (x != 0 && y != 0)
//                     g = currentCell.g + 14;
//                 else
//                     g = currentCell.g + 10;

//                 int h = std::abs((endingPos.x - childPos.x) * 10) + std::abs((endingPos.y - childPos.y) * 4);;
//                 int f = g + h;

//                 bool foundInOpen = false;
//                 for (Cell &cell : openList) {
//                     if (cell.pos.x = childPos.x && cell.pos.y == childPos.y) {
//                         if (cell.f > f) {
//                             cell.f = f;
//                             cell.g = g;
//                             cell.originIndex = currentCellIndex;
//                         }

//                         foundInOpen = true;
//                         break;
//                     }
//                 }

//                 if (foundInOpen)
//                     continue;
                
//                 openList.push_back(Cell {g, f, currentCellIndex, childPos});
//             }
//         }

//     }
// }

// void UpdatePathFinding(int maxIterations) {
//     Vector2 endingPos = {(float) grid.width - 1, (float) grid.height - 1};

//     for (int i = 0; i < maxIterations; i++) {
//         // End Search If there is not where to go
//         if (!openList.size()) {
//             isPathFinding = false;
//             finishedPathFinding = false;
//             print("The path is blocked :(");
//             return;
//         }

//         // Find the cell with the smallest f value
//         int bestCellIndex = 0;
//         for (int index = 0; index < (signed) openList.size(); index++) {
//             if (openList[index].f < openList[bestCellIndex].f)
//                 bestCellIndex = index;
//         }

//         Cell bestCell = openList[bestCellIndex];

//         // Add the best cell to the closed lost
//         closedList.push_back(bestCell);

//         // Remove the best cell from the open list
//         openList.erase(openList.begin() + bestCellIndex);
        
//         // Loop through neighboring cells
//         for (int x = -1; x < 2; x++) {
//             for (int y = -1; y < 2; y++) {
//                 if (x == 0 && y == 0) continue;  // Don't check the middle square
                
//                 // Get the current cell position
//                 Vector2 cellPos = {bestCell.pos.x + x, bestCell.pos.y + y};

//                 // Check if the child cell position is invalid
//                 if (grid.GetAt(cellPos.x, cellPos.y) != 0) continue;

//                 bool foundInClosed = false;
//                 for (Cell &cell : closedList) {
//                     if (cell.pos.x = cellPos.x && cell.pos.y == cellPos.y) {
//                         foundInClosed = true;
//                         break;
//                     }
//                 }

//                 if (foundInClosed)
//                     continue;

//                 // If we have found the cell
//                 if (cellPos.x == endingPos.x && cellPos.y == endingPos.y) {
//                     print("Mission successful folks! :D");
//                     isPathFinding = false;
//                     finishedPathFinding = true;
//                     finalPath.push_back(cellPos);

//                     // Construct the final path
//                     int currentIndex = (signed) closedList.size() - 1;
//                     while (true) {
//                         // Add the origin pos
//                         finalPath.insert(finalPath.begin(), closedList[currentIndex].pos);

//                         // Check if we are at the start
//                         if (currentIndex == 0) break;

//                         // Set the new index;
//                         currentIndex = closedList[currentIndex].originIndex;
//                     }

//                     return;
//                 }

//                 // Find f value
//                 int g = bestCell.g + (x != 0 && y != 0 ? 14 : 10);

//                 int xDistance = std::abs(cellPos.x - endingPos.x);
//                 int yDistance = std::abs(cellPos.y - endingPos.y);
//                 int h = xDistance > yDistance ? (xDistance * 10 + yDistance * 4) : (xDistance * 4 + yDistance * 10);
//                 int f = g + h;
                
//                 bool foundInOpen = false;
//                 for (Cell &cell : openList) {
//                     if (cell.pos.x = cellPos.x && cell.pos.y == cellPos.y && cell.g < g) {
//                         foundInOpen = true;
//                         break;
//                     }
//                 }

//                 if (foundInOpen)
//                     continue;

//                 // Add the cell to the open list
//                 openList.push_back(Cell {f, g, (signed) closedList.size() - 1, cellPos});
//             }
//         }
//     }
// }
