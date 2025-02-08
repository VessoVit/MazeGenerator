#include "MazeGenerator.h"

MazeGenerator::MazeGenerator(int width, int height) 
    : mazeWidth(width), mazeHeight(height), animating(false) {
    directions = {{0, 2}, {2, 0}, {0, -2}, {-2, 0}};
}

void MazeGenerator::generate(vector<vector<int>>& maze) {
    // Start with all walls
    for (int y = 0; y < 2 * mazeHeight + 1; y++) {
        for (int x = 0; x < 2 * mazeWidth + 1; x++) {
            maze[y][x] = 1;
        }
    }
    
    // Start at a random cell
    current_x = 2 * (static_cast<int>(ofRandom(mazeWidth))) + 1;
    current_y = 2 * (static_cast<int>(ofRandom(mazeHeight))) + 1;
    maze[current_y][current_x] = 0;
    
    unvisited = mazeWidth * mazeHeight - 1;
    
    while (unvisited > 0) {
        // Pick a random direction
        int dir_idx = static_cast<int>(ofRandom(4));
        int dx = directions[dir_idx].first;
        int dy = directions[dir_idx].second;
        int next_x = current_x + dx;
        int next_y = current_y + dy;
        
        if (isValid(next_x, next_y) && maze[next_y][next_x] == 1) {
            maze[(current_y + next_y) / 2][(current_x + next_x) / 2] = 0;
            maze[next_y][next_x] = 0;
            unvisited--;
            current_x = next_x;
            current_y = next_y;
        } else {
            do {
                current_x = 2 * (static_cast<int>(ofRandom(mazeWidth))) + 1;
                current_y = 2 * (static_cast<int>(ofRandom(mazeHeight))) + 1;
            } while (maze[current_y][current_x] == 1);
        }
    }
    
    // Create entrance and exit
    maze[0][1] = 0;
    maze[2 * mazeHeight][2 * mazeWidth - 1] = 0;
    maze[1][1] = 0;
    maze[2 * mazeHeight - 1][2 * mazeWidth - 1] = 0;
}

void MazeGenerator::updateAnimation(vector<vector<int>>& maze) {
    if (!animating || unvisited <= 0) return;
    
    int dir_idx = static_cast<int>(ofRandom(4));
    int dx = directions[dir_idx].first;
    int dy = directions[dir_idx].second;
    int next_x = current_x + dx;
    int next_y = current_y + dy;
    
    if (isValid(next_x, next_y) && maze[next_y][next_x] == 1) {
        maze[(current_y + next_y) / 2][(current_x + next_x) / 2] = 0;
        maze[next_y][next_x] = 0;
        unvisited--;
        current_x = next_x;
        current_y = next_y;
    } else {
        do {
            current_x = 2 * (static_cast<int>(ofRandom(mazeWidth))) + 1;
            current_y = 2 * (static_cast<int>(ofRandom(mazeHeight))) + 1;
        } while (maze[current_y][current_x] == 1);
    }
    
    if (unvisited <= 0) {
        maze[0][1] = 0;
        maze[2 * mazeHeight][2 * mazeWidth - 1] = 0;
        maze[1][1] = 0;
        maze[2 * mazeHeight - 1][2 * mazeWidth - 1] = 0;
        animating = false;
    }
}

void MazeGenerator::reset() {
    current_x = 2 * (static_cast<int>(ofRandom(mazeWidth))) + 1;
    current_y = 2 * (static_cast<int>(ofRandom(mazeHeight))) + 1;
    unvisited = mazeWidth * mazeHeight - 1;
    animating = true;
}

bool MazeGenerator::isValid(int x, int y) const {
    return x > 0 && x < 2 * mazeWidth && y > 0 && y < 2 * mazeHeight;
}
