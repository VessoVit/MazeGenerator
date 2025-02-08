#include "MazeSolver.h"
#include <deque>
#include <set>

MazeSolver::MazeSolver() {}

void MazeSolver::solve(const vector<vector<int>>& maze, int width, int height) {
    solution.clear();
    
    pair<int, int> start = {1, 0};
    pair<int, int> end = {2 * width - 1, 2 * height};
    
    deque<pair<pair<int, int>, vector<pair<int, int>>>> queue;
    set<pair<int, int>> visited;
    
    queue.push_back({start, {start}});
    visited.insert(start);
    
    vector<pair<int, int>> moves = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    
    while (!queue.empty()) {
        auto current = queue.front().first;
        auto path = queue.front().second;
        queue.pop_front();
        
        if (current == end) {
            solution = path;
            return;
        }
        
        for (const auto& move : moves) {
            int next_x = current.first + move.first;
            int next_y = current.second + move.second;
            
            if (isValid(next_x, next_y, width, height) && 
                maze[next_y][next_x] == 0 && 
                visited.find({next_x, next_y}) == visited.end()) {
                
                auto new_path = path;
                new_path.push_back({next_x, next_y});
                queue.push_back({{next_x, next_y}, new_path});
                visited.insert({next_x, next_y});
            }
        }
    }
}

bool MazeSolver::isValid(int x, int y, int width, int height) const {
    return x >= 0 && x < (2 * width + 1) && y >= 0 && y < (2 * height + 1);
}
