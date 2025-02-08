#pragma once
#include "ofMain.h"

class MazeGenerator {
public:
    MazeGenerator(int width, int height);
    void generate(vector<vector<int>>& maze);
    bool isAnimating() const { return animating; }
    void updateAnimation(vector<vector<int>>& maze);
    void reset();
    
    // Animation properties
    bool animating;
    int current_x;
    int current_y;
    int unvisited;
    vector<pair<int, int>> directions;

private:
    int mazeWidth;
    int mazeHeight;
    bool isValid(int x, int y) const;
};
