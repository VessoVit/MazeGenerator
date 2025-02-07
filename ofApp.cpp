#include "ofApp.h"
#include <deque>
#include <random>
#include <chrono>

//--------------------------------------------------------------
void ofApp::setup() {
    cellSize = 20;  // Initial cell size
    ofSetFrameRate(60);
    showSolution = false;
    
    // Setup GUI
    gui.setup("Maze Controls");
    showGui.set("Show GUI", true);
    mazeInfo.set("Maze Info", "");
    
    // Maze Settings group
    sizeControls.setName("Maze Settings");
    animationEnabled.set("Enable Animation", true);
    view3D.set("3D View", false);
    cellSizeGui.set("Cell Size", cellSize, 10, 50);
    sizeControls.add(animationEnabled);
    sizeControls.add(view3D);
    sizeControls.add(cellSizeGui);
    
    // Initialize 3D properties
    wallHeight = cellSize * 2;
    cam.setDistance(500);
    
    // Setup lights
    pointLight.setPointLight();
    pointLight.setDiffuseColor(ofColor(255, 255, 255));
    pointLight.setSpecularColor(ofColor(255, 255, 255));
    
    directionalLight.setDirectional();
    directionalLight.setDiffuseColor(ofColor(200, 200, 200));
    directionalLight.setSpecularColor(ofColor(255, 255, 255));
    generateButton.setup("Generate New Maze");
    solveButton.setup("Show Solution");  // Changed text to be more clear
    gui.add(&generateButton);
    gui.add(&solveButton);
    
    // Algorithm selection group
    algorithmGroup.setName("Generation Algorithm");
    algorithmRecursive.set("Recursive Backtracker", true);
    algorithmPrims.set("Prim's Algorithm", false);
    algorithmKruskals.set("Kruskal's Algorithm", false);
    algorithmGroup.add(algorithmRecursive);
    algorithmGroup.add(algorithmPrims);
    algorithmGroup.add(algorithmKruskals);
    
    // Add groups to GUI
    gui.add(sizeControls);
    gui.add(algorithmGroup);
    
    generateButton.addListener(this, &ofApp::onGeneratePressed);
    solveButton.addListener(this, &ofApp::onSolvePressed);
    
    currentGenerationAlgorithm = GenerationAlgorithm::RECURSIVE_BACKTRACKER;
    
    // Set initial window size
    ofSetWindowShape(600, 600);
    
    // Initialize maze dimensions
    mazeWidth = 10;  // Start with fixed size for testing
    mazeHeight = 10;
    
    // Initialize maze vector with proper dimensions
    maze.resize(2 * mazeHeight + 1);
    for (auto& row : maze) {
        row.resize(2 * mazeWidth + 1, 1);
    }
    
    // Initialize animation properties
    animatingGeneration = false;
    animatingSolution = false;
    generationDelay = 50;  // milliseconds
    solutionDelay = 100;   // milliseconds
    lastUpdateTime = 0;
    
    // Initialize directions for maze generation
    directions = {{0, 2}, {2, 0}, {0, -2}, {-2, 0}};
    
    // Generate first maze
    resetMaze();
    generateMaze();
}

void ofApp::onGeneratePressed() {
    if (!animatingGeneration) {
        resetMaze();
        if (animationEnabled) {
            // Start animated generation
            animatingGeneration = true;
            animatingSolution = false;
            showSolution = false;
            current_x = 2 * (static_cast<int>(ofRandom(mazeWidth))) + 1;
            current_y = 2 * (static_cast<int>(ofRandom(mazeHeight))) + 1;
            unvisited = mazeWidth * mazeHeight - 1;
            maze[current_y][current_x] = 0;
        } else {
            // Instant generation
            animatingGeneration = false;
            generateMaze();
            solveMaze();
        }
    } else if (animatingGeneration) {
        // Stop animation if it's running
        animatingGeneration = false;
        generateMaze();
        solveMaze();
    }
}

void ofApp::onSolvePressed() {
    if (!animatingGeneration) {
        // Always toggle solution visibility
        showSolution = !showSolution;
        
        if (showSolution) {
            // If showing solution and animation is enabled, start animation
            if (animationEnabled) {
                animatingSolution = true;
                solution.clear();
                solveMaze();
                currentSolutionIndex = 0;
            } else {
                // Just show the full solution without animation
                solution.clear();
                solveMaze();
                animatingSolution = false;
            }
        } else {
            // When hiding solution, stop any ongoing animation
            animatingSolution = false;
        }
    }
}

//--------------------------------------------------------------
void ofApp::update() {
    float currentTime = ofGetElapsedTimeMillis();
    
    // Handle algorithm selection
    if (algorithmRecursive && currentGenerationAlgorithm != GenerationAlgorithm::RECURSIVE_BACKTRACKER) {
        algorithmPrims = false;
        algorithmKruskals = false;
        currentGenerationAlgorithm = GenerationAlgorithm::RECURSIVE_BACKTRACKER;
    } else if (algorithmPrims && currentGenerationAlgorithm != GenerationAlgorithm::PRIMS) {
        algorithmRecursive = false;
        algorithmKruskals = false;
        currentGenerationAlgorithm = GenerationAlgorithm::PRIMS;
    } else if (algorithmKruskals && currentGenerationAlgorithm != GenerationAlgorithm::KRUSKALS) {
        algorithmRecursive = false;
        algorithmPrims = false;
        currentGenerationAlgorithm = GenerationAlgorithm::KRUSKALS;
    }
    
    // Sync cell size with GUI
    if (cellSize != cellSizeGui) {
        cellSize = cellSizeGui;
        updateMazeDimensions();
        resetMaze();
        generateMaze();
        solveMaze();
    }
    
    if ((animatingGeneration || animatingSolution) && 
        (currentTime - lastUpdateTime > (animatingGeneration ? generationDelay : solutionDelay))) {
        updateAnimation();
        lastUpdateTime = currentTime;
    } else if (animatingSolution && !solution.empty()) {
        // Animate solution path
        if (currentSolutionIndex < static_cast<int>(solution.size())) {
            currentSolutionIndex++;
        } else {
            animatingSolution = false;
        }
    }
    
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofBackground(255);
    
    // Update maze info
    string info = "Maze Size: " + ofToString(mazeWidth) + "x" + ofToString(mazeHeight) + "\n";
    info += "Cell Size: " + ofToString(cellSize) + "px\n";
    info += "Generation Algorithm: ";
    info += (algorithmRecursive ? "Recursive Backtracker" :
             algorithmPrims ? "Prim's Algorithm" :
             "Kruskal's Algorithm");
    info += "\n";
    info += animatingGeneration ? "Generating..." :
    (animatingSolution ? "Solving..." : "Ready");
    mazeInfo.set(info);
    
    if (view3D) {
        ofEnableLighting();
        ofEnableDepthTest();
        
        cam.begin();
        
        // Position and enable lights
        pointLight.setPosition(0, 0, 500);
        pointLight.enable();
        directionalLight.setPosition(200, 200, 200);
        directionalLight.enable();
        
        // Center the maze
        ofTranslate(
                    -(2 * mazeWidth + 1) * cellSize / 2,
                    -(2 * mazeHeight + 1) * cellSize / 2,
                    0
                    );
        
        // Draw maze
        // Draw floor
        ofSetColor(200);
        ofDrawRectangle(0, 0, (2 * mazeWidth + 1) * cellSize, (2 * mazeHeight + 1) * cellSize);
        
        // Draw walls
        ofSetColor(100, 100, 120);  // Light gray-blue color for better contrast
        for (int y = 0; y < 2 * mazeHeight + 1; y++) {
            for (int x = 0; x < 2 * mazeWidth + 1; x++) {
                if (maze[y][x] == 1) {
                    ofDrawBox(
                              x * cellSize + cellSize/2,
                              y * cellSize + cellSize/2,
                              wallHeight/2,
                              cellSize,
                              cellSize,
                              wallHeight
                              );
                }
            }
        }
    } else {
        for (int y = 0; y < 2 * mazeHeight + 1; y++) {
            for (int x = 0; x < 2 * mazeWidth + 1; x++) {
                if (maze[y][x] == 1) {
                    drawCell(x, y, ofColor(0));  // Wall
                }
            }
        }
    }
    
    // Draw current position during generation
    if (animatingGeneration) {
        drawCell(current_x, current_y, ofColor(255, 0, 0, 128));  // Semi-transparent red
    }
    
    // Draw solution if enabled and exists
    if (showSolution && !solution.empty()) {
        if (view3D) {
            // Draw solution path slightly above floor
            ofSetColor(255, 0, 0);
            ofSetLineWidth(cellSize/3);
            
            int endIndex = animatingSolution ? currentSolutionIndex : solution.size();
            for (size_t i = 0; i < endIndex - 1 && i < solution.size() - 1; i++) {
                const auto& current = solution[i];
                const auto& next = solution[i + 1];
                
                float x1 = (current.first + 0.5) * cellSize;
                float y1 = (current.second + 0.5) * cellSize;
                float x2 = (next.first + 0.5) * cellSize;
                float y2 = (next.second + 0.5) * cellSize;
                
                ofDrawLine(x1, y1, cellSize/2, x2, y2, cellSize/2);
            }
        } else {
            // Draw solution path background
            ofSetColor(255, 240, 240);  // Light red background
            int endIndex = animatingSolution ? currentSolutionIndex : solution.size();
            for (int i = 0; i < endIndex && i < solution.size(); i++) {
                const auto& pos = solution[i];
                drawCell(pos.first, pos.second, ofColor(255, 240, 240));
            }
            ofSetColor(255, 0, 0);  // Red path
            ofSetLineWidth(cellSize/3);
            
            // Draw lines connecting solution points
            int lineEndIndex = animatingSolution ? currentSolutionIndex : solution.size();
            for (size_t i = 0; i < lineEndIndex - 1 && i < solution.size() - 1; i++) {
                const auto& current = solution[i];
                const auto& next = solution[i + 1];
                
                float x1 = (current.first + 0.5) * cellSize;
                float y1 = (current.second + 0.5) * cellSize;
                float x2 = (next.first + 0.5) * cellSize;
                float y2 = (next.second + 0.5) * cellSize;
                
                ofDrawLine(x1, y1, x2, y2);
            }
        }
        
    }
    
    if (view3D) {
        cam.end();
        ofDisableLighting();
        ofDisableDepthTest();
    }
    
    // Draw GUI if enabled
    if (showGui) {
        gui.draw();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    bool needsUpdate = false;
    
    if (key == ' ') {  // Spacebar generates new maze instantly
        animatingGeneration = false;  // Stop any ongoing animation
        animatingSolution = false;
        resetMaze();
        generateMaze();
        solveMaze();
        animationEnabled = false;  // Disable animation when using spacebar
    } else if (key == 'g') {  // 'g' toggles animated generation
        animationEnabled = !animationEnabled;  // Toggle animation state
        if (animatingGeneration) {
            // Stop current animation if running
            animatingGeneration = false;
            generateMaze();
            solveMaze();
        }
    } else if (key == 'f') {  // 'f' toggles animated solution
        if (!animatingGeneration) {
            if (!animatingSolution) {
                animatingSolution = true;
                showSolution = true;
                solution.clear();
                solveMaze();
                currentSolutionIndex = 0;
            } else {
                animatingSolution = false;
            }
        }
    } else if (key == 's') {  // 's' toggles solution visibility
        showSolution = !showSolution;
        animatingSolution = false;  // Stop any ongoing solution animation
    } else if (key == '+' || key == '=') {  // Increase cell size
        cellSizeGui = min(50, cellSize + 2);
        cellSize = cellSizeGui;
        needsUpdate = true;
    } else if (key == '-' || key == '_') {  // Decrease cell size
        cellSizeGui = max(10, cellSize - 2);
        cellSize = cellSizeGui;
        needsUpdate = true;
    } else if (key == 'h') {  // Toggle GUI
        showGui = !showGui;
    }
    
    if (needsUpdate) {
        updateMazeDimensions();
        resetMaze();
        generateMaze();
        solveMaze();
    }
}

//--------------------------------------------------------------
void ofApp::generateMaze() {
    // Start with all walls
    for (int y = 0; y < 2 * mazeHeight + 1; y++) {
        for (int x = 0; x < 2 * mazeWidth + 1; x++) {
            maze[y][x] = 1;
        }
    }
    
    // Start at a random cell
    int current_x = 2 * (static_cast<int>(ofRandom(mazeWidth))) + 1;
    int current_y = 2 * (static_cast<int>(ofRandom(mazeHeight))) + 1;
    maze[current_y][current_x] = 0;
    
    int unvisited = mazeWidth * mazeHeight - 1;
    vector<pair<int, int>> directions = {{0, 2}, {2, 0}, {0, -2}, {-2, 0}};
    
    while (unvisited > 0) {
        // Pick a random direction
        int dir_idx = static_cast<int>(ofRandom(4));
        int dx = directions[dir_idx].first;
        int dy = directions[dir_idx].second;
        int next_x = current_x + dx;
        int next_y = current_y + dy;
        
        // Check if the next cell is valid and unvisited
        if (next_x > 0 && next_x < 2 * mazeWidth && 
            next_y > 0 && next_y < 2 * mazeHeight && 
            maze[next_y][next_x] == 1) {
            // Carve a passage
            maze[(current_y + next_y) / 2][(current_x + next_x) / 2] = 0;  // Remove wall
            maze[next_y][next_x] = 0;  // Mark cell as visited
            unvisited--;
            current_x = next_x;
            current_y = next_y;
        } else {
            // If we hit a wall or visited cell, randomly jump to a visited cell
            do {
                current_x = 2 * (static_cast<int>(ofRandom(mazeWidth))) + 1;
                current_y = 2 * (static_cast<int>(ofRandom(mazeHeight))) + 1;
            } while (maze[current_y][current_x] == 1);
        }
    }
    
    // Create entrance and exit
    maze[0][1] = 0;  // Entrance at top
    maze[2 * mazeHeight][2 * mazeWidth - 1] = 0;  // Exit at bottom
    
    // Ensure the cells next to entrance/exit are open
    maze[1][1] = 0;  // Cell after entrance
    maze[2 * mazeHeight - 1][2 * mazeWidth - 1] = 0;  // Cell before exit
}

//--------------------------------------------------------------
bool ofApp::isValid(int x, int y) {
    // Check bounds for the entire maze including walls
    return x >= 0 && x < (2 * mazeWidth + 1) && 
           y >= 0 && y < (2 * mazeHeight + 1);
}

//--------------------------------------------------------------
void ofApp::solveMaze() {
    solution.clear();
    
    // Start from entrance, end at exit
    pair<int, int> start = {1, 0};
    pair<int, int> end = {2 * mazeWidth - 1, 2 * mazeHeight};
    
    // BFS queue and visited set
    deque<pair<pair<int, int>, vector<pair<int, int>>>> queue;
    set<pair<int, int>> visited;
    
    // Initialize with start position
    queue.push_back({start, {start}});
    visited.insert(start);
    
    // Possible moves: down, right, up, left
    vector<pair<int, int>> moves = {{0, 1}, {1, 0}, {0, -1}, {-1, 0}};
    
    while (!queue.empty()) {
        auto current = queue.front().first;
        auto path = queue.front().second;
        queue.pop_front();
        
        // Check if we reached the exit
        if (current == end) {
            solution = path;
            return;
        }
        
        // Try all possible moves
        for (const auto& move : moves) {
            int next_x = current.first + move.first;
            int next_y = current.second + move.second;
            
            // Check if the move is valid
            if (isValid(next_x, next_y) && 
                maze[next_y][next_x] == 0 && 
                visited.find({next_x, next_y}) == visited.end()) {
                
                // Create new path with this move
                auto new_path = path;
                new_path.push_back({next_x, next_y});
                
                // Add to queue and mark as visited
                queue.push_back({{next_x, next_y}, new_path});
                visited.insert({next_x, next_y});
            }
        }
    }
}

//--------------------------------------------------------------
void ofApp::drawCell(int x, int y, const ofColor& color) {
    ofSetColor(color);
    ofFill();
    ofDrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize);
}

//--------------------------------------------------------------
void ofApp::resetMaze() {
    // Ensure maze vector has correct size before accessing
    if (maze.size() != 2 * mazeHeight + 1) {
        maze.resize(2 * mazeHeight + 1);
    }
    for (int y = 0; y < 2 * mazeHeight + 1; y++) {
        if (maze[y].size() != 2 * mazeWidth + 1) {
            maze[y].resize(2 * mazeWidth + 1);
        }
        for (int x = 0; x < 2 * mazeWidth + 1; x++) {
            maze[y][x] = 1;
        }
    }
    solution.clear();
}
void ofApp::windowResized(int w, int h) {
    updateMazeDimensions();
    resetMaze();
    generateMaze();
    solveMaze();
}

void ofApp::updateMazeDimensions() {
    // Calculate new dimensions
    int newWidth = max(5, (ofGetWidth() / cellSize - 1) / 2);
    int newHeight = max(5, (ofGetHeight() / cellSize - 1) / 2);
    
    // Only update if dimensions have changed
    if (newWidth != mazeWidth || newHeight != mazeHeight) {
        mazeWidth = newWidth;
        mazeHeight = newHeight;
        
        // Properly resize both dimensions of the vector
        maze.resize(2 * mazeHeight + 1);
        for (auto& row : maze) {
            row.resize(2 * mazeWidth + 1, 1);
        }
        
        // Clear solution when dimensions change
        solution.clear();
    }
}
void ofApp::updateAnimation() {
    if (animatingGeneration) {
        if (unvisited > 0) {
            // Pick a random direction
            int dir_idx = static_cast<int>(ofRandom(4));
            int dx = directions[dir_idx].first;
            int dy = directions[dir_idx].second;
            int next_x = current_x + dx;
            int next_y = current_y + dy;
            
            // Check if the next cell is valid and unvisited
            if (next_x > 0 && next_x < 2 * mazeWidth && 
                next_y > 0 && next_y < 2 * mazeHeight && 
                maze[next_y][next_x] == 1) {
                // Carve a passage
                maze[(current_y + next_y) / 2][(current_x + next_x) / 2] = 0;
                maze[next_y][next_x] = 0;
                unvisited--;
                current_x = next_x;
                current_y = next_y;
            } else {
                // If we hit a wall or visited cell, randomly jump to a visited cell
                do {
                    current_x = 2 * (static_cast<int>(ofRandom(mazeWidth))) + 1;
                    current_y = 2 * (static_cast<int>(ofRandom(mazeHeight))) + 1;
                } while (maze[current_y][current_x] == 1);
            }
        } else {
            // Finish generation
            maze[0][1] = 0;  // Create entrance
            maze[2 * mazeHeight][2 * mazeWidth - 1] = 0;  // Create exit
            maze[1][1] = 0;  // Ensure entrance path
            maze[2 * mazeHeight - 1][2 * mazeWidth - 1] = 0;  // Ensure exit path
            animatingGeneration = false;
        }
    }
}
