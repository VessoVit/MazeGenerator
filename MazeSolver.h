#pragma once
#include "ofMain.h"

class MazeSolver {
public:
    MazeSolver();
    void solve(const vector<vector<int>>& maze, int width, int height);
    const vector<pair<int, int>>& getSolution() const { return solution; }
    void clear() { solution.clear(); }
    
private:
    vector<pair<int, int>> solution;
    bool isValid(int x, int y, int width, int height) const;
    ~MazeSolver() = default;
};
