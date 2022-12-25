#pragma once
#include <vector>

struct Cell {
    int g;
    int f;
    int parentIndex;
    Vector2 pos;
};

class Grid {
public:
    int width;
    int height;
    std::vector<int> values;

    Grid() = default;
    Grid(int _width, int _height);

    void SetAt(int x, int y, int newValue);
    void Clear();
    int GetAt(int x, int y);
    int IndexAt(int x, int y);
    bool IsOnBoard(int x, int y);
};