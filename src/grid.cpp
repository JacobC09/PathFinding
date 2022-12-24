#include "grid.h"

Grid::Grid(int _width, int _height) {
    width = _width;
    height = _height;
    values = std::vector<int>(width * height, 0);
}

void Grid::SetAt(int x, int y, int newValue) {
    if (!IsOnBoard(x, y)) return;
    
    values[IndexAt(x, y)] = newValue;
}

void Grid::Clear() {
    for (int index = 0; index < (signed) values.size(); index++) {
        values[index] = 0;
    }
}

int Grid::GetAt(int x, int y) {
    if (!IsOnBoard(x, y)) return -1;
    
    return values[IndexAt(x, y)];
}

int Grid::IndexAt(int x, int y) {
    return y * width + x;
}

bool Grid::IsOnBoard(int x, int y) {
    return x >= 0 && y >= 0 && x < width && y < height;
}
