#pragma once

#include "ofMain.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void updateAnimation();
    
    // Maze properties
    int cellSize;
    int mazeWidth;
    int mazeHeight;
    
    vector<vector<int>> maze;
    vector<pair<int, int>> solution;
    
    // Animation properties
    bool animatingGeneration;
    bool animatingSolution;
    int generationDelay;
    int solutionDelay;
    float lastUpdateTime;
    int currentSolutionIndex;
    
    // Generation animation state
    int current_x;
    int current_y;
    int unvisited;
    vector<pair<int, int>> directions;
    
    // Maze generation methods
    void generateMaze();
    bool isValid(int x, int y);
    void solveMaze();
    
    // Helper methods
    void drawCell(int x, int y, const ofColor& color);
    void resetMaze();
    
    // Window event handlers
    void windowResized(int w, int h);
    
private:
    bool showSolution;
    void updateMazeDimensions();
    
    // GUI
    ofxPanel gui;
    ofParameter<bool> showGui;
    ofParameter<string> currentAlgorithm;
    ofParameter<string> mazeInfo;
    ofParameter<int> cellSizeGui;
    ofParameterGroup algorithms;
    ofxButton generateButton;
    ofxButton solveButton;
    
    // Algorithm types
    enum class GenerationAlgorithm {
        RECURSIVE_BACKTRACKER,
        PRIMS,
        KRUSKALS
    };
    GenerationAlgorithm currentGenerationAlgorithm;
};
