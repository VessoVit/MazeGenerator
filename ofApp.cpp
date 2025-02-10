#include "ofApp.h"
#include <deque>
#include <random>
#include <chrono>

//--------------------------------------------------------------
void ofApp::setup() {
    cellSize = 20;  // Initial cell size
    ofSetFrameRate(120);
    showSolution = false;
//    ofSetCurrentRenderer(std::shared_ptr<ofBaseRenderer> renderer);
    
    
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
    wallHeight = cellSize * 2;  // Reduced wall height
    cam.setDistance(500);
    cam.setNearClip(0.001);    // Allow even closer zoom
    cam.setFarClip(10000);     // Maintain far viewing distance
    cam.setTarget(ofVec3f(0, 0, 0)); // Set camera target to center
    
    // Setup lights for OF 0.12
    pointLight.setup();
    pointLight.enable();
    pointLight.setDiffuseColor(ofColor(200, 200, 200));  // Softer diffuse light
    pointLight.setSpecularColor(ofColor(255, 255, 255));
    pointLight.setAttenuation(1.0, 0.002, 0.0);  // Adjusted attenuation for better falloff
    
    directionalLight.setup();
    directionalLight.enable();
    directionalLight.setDirectional();
    directionalLight.setDiffuseColor(ofColor(100, 100, 100));  // Dimmer ambient light
    directionalLight.setSpecularColor(ofColor(150, 150, 150));  // Reduced specular
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
    ofSetWindowShape(1024, 768);
    
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
    ofBackground(33);  // Black background
    
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
        ofEnableDepthTest();
        ofEnableLighting();
        
        // Set up camera for better 3D viewing
        cam.disableOrtho();  // Use perspective for better 3D view
        cam.begin();
        
        // Adjust lighting for better visibility
        ofEnableSeparateSpecularLight();
        pointLight.setDiffuseColor(ofColor(255, 255, 255));  // Brighter diffuse light
        pointLight.setSpecularColor(ofColor(255, 255, 255)); // Full specular
        pointLight.setPosition((2 * mazeWidth + 1) * cellSize / 2,  // Center light above maze
                             (2 * mazeHeight + 1) * cellSize / 2,
                             wallHeight * 3);
        
        
        // Scale for retina displays
        float scale = ofGetScreenWidth() / ofGetWidth();
        ofScale(scale, scale, scale);
        
        // Update directional light for ambient illumination
        directionalLight.setPosition(0, 0, wallHeight * 4);
        directionalLight.setOrientation(ofVec3f(0, 0, -90));
        directionalLight.setDiffuseColor(ofColor(150, 150, 150));  // Brighter ambient light
        
        // Center the maze
        ofTranslate(
                    -(2 * mazeWidth + 1) * cellSize / 2,
                    -(2 * mazeHeight + 1) * cellSize / 2,
                    0
                    );
        
        // Draw maze
        // Draw floor
        ofSetColor(50);  // Darker floor for better contrast
        ofDrawRectangle(0, 0, (2 * mazeWidth + 1) * cellSize, (2 * mazeHeight + 1) * cellSize);
        
        // Draw walls as a single solid structure
        ofSetColor(100, 100, 120);  // Light gray-blue color for better contrast
        ofMesh wallMesh;
        wallMesh.setMode(OF_PRIMITIVE_TRIANGLES);
        
        // Helper lambda to add a wall face (two triangles)
        auto addWallFace = [&](const ofVec3f& bl, const ofVec3f& br, 
                              const ofVec3f& tr, const ofVec3f& tl) {
            int idx = wallMesh.getNumVertices();
            wallMesh.addVertex(bl);
            wallMesh.addVertex(br);
            wallMesh.addVertex(tr);
            wallMesh.addVertex(tl);
            
            // First triangle
            wallMesh.addIndex(idx);
            wallMesh.addIndex(idx + 1);
            wallMesh.addIndex(idx + 2);
            
            // Second triangle
            wallMesh.addIndex(idx);
            wallMesh.addIndex(idx + 2);
            wallMesh.addIndex(idx + 3);
            
            // Add normals for better lighting
            ofVec3f normal = ((br - bl).getCrossed(tr - bl)).normalized();
            for (int i = 0; i < 4; i++) {
                wallMesh.addNormal(normal);
            }
        };
        
        for (int y = 0; y < 2 * mazeHeight + 1; y++) {
            for (int x = 0; x < 2 * mazeWidth + 1; x++) {
                if (maze[y][x] == 1) {
                    float wx = x * cellSize;
                    float wy = y * cellSize;
                    float wz = 0;
                    
                    // Only create faces that are visible (not adjacent to another wall)
                    bool hasWallNorth = (y > 0) && maze[y-1][x] == 1;
                    bool hasWallSouth = (y < 2 * mazeHeight) && maze[y+1][x] == 1;
                    bool hasWallEast = (x < 2 * mazeWidth) && maze[y][x+1] == 1;
                    bool hasWallWest = (x > 0) && maze[y][x-1] == 1;

                    // Create vertices for the wall cube
                    ofVec3f frontBL(wx, wy, wz);
                    ofVec3f frontBR(wx + cellSize, wy, wz);
                    ofVec3f frontTR(wx + cellSize, wy, wz + wallHeight);
                    ofVec3f frontTL(wx, wy, wz + wallHeight);
                    
                    ofVec3f backBL(wx, wy + cellSize, wz);
                    ofVec3f backBR(wx + cellSize, wy + cellSize, wz);
                    ofVec3f backTR(wx + cellSize, wy + cellSize, wz + wallHeight);
                    ofVec3f backTL(wx, wy + cellSize, wz + wallHeight);
                    
                    // Only add faces that are visible
                    if (!hasWallNorth) addWallFace(frontBL, frontBR, frontTR, frontTL); // Front
                    if (!hasWallSouth) addWallFace(backBR, backBL, backTL, backTR);     // Back
                    if (!hasWallEast) addWallFace(frontBR, backBR, backTR, frontTR);   // Right
                    if (!hasWallWest) addWallFace(backBL, frontBL, frontTL, backTL);   // Left
                    addWallFace(frontTL, frontTR, backTR, backTL);   // Top always visible
                }
            }
        }
        
        // Draw the entire maze as a single mesh
        wallMesh.draw();
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
            // Create glowing material for the solution path
            ofMaterial material;
            material.setEmissiveColor(ofColor(255, 140, 0, 255));  // Strong golden orange emission
            material.setDiffuseColor(ofColor(255));  // No diffuse color to enhance glow effect
            material.setSpecularColor(ofColor(255, 200, 0));  // Golden specular highlight
            material.setShininess(128);  // High shininess for glow effect
            material.begin();
            
            // Draw solution path as a continuous tube
            int endIndex = animatingSolution ? currentSolutionIndex : solution.size();
            if (endIndex > 1) {
                ofMesh tubeMesh;
                tubeMesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
                const int segments = 8; // Number of segments around the tube
                const float radius = cellSize/4;  // Much thicker tube
                
                for (size_t i = 0; i < endIndex - 1; i++) {
                    const auto& current = solution[i];
                    const auto& next = solution[i + 1];
                    
                    float x1 = (current.first + 0.5) * cellSize;
                    float y1 = (current.second + 0.5) * cellSize;
                    float x2 = (next.first + 0.5) * cellSize;
                    float y2 = (next.second + 0.5) * cellSize;
                    
                    // Calculate direction vector
                    ofVec3f dir(x2 - x1, y2 - y1, 0);
                    ofVec3f up(0, 0, 1);
                    ofVec3f right = dir.getCrossed(up).normalized();
                    
                    // Create circle points around the tube
                    for (int s = 0; s <= segments; s++) {
                        float angle = TWO_PI * s / segments;
                        float cosA = cos(angle);
                        float sinA = sin(angle);
                        
                        // Calculate points for both ends of this segment
                        ofVec3f offset = right * (radius * cosA) + up * (radius * sinA);
                        
                        // Add vertices for both ends with proper height (lifted by half cell size)
                        ofVec3f v1(x1, y1, cellSize);  // Lifted to cellSize instead of cellSize/2
                        ofVec3f v2(x2, y2, cellSize);  // Lifted to cellSize instead of cellSize/2
                        
                        // Calculate normal vector for proper 3D appearance
                        ofVec3f normal = offset.normalized();
                        
                        // Add height offset based on normal
                        v1 += normal * radius;
                        v2 += normal * radius;
                        
                        tubeMesh.addVertex(v1);
                        tubeMesh.addVertex(v2);
                        
                        // Add color for both vertices
                        ofFloatColor color(1.0, 0.55, 0.0);  // Golden orange
                        tubeMesh.addColor(color);
                        tubeMesh.addColor(color);
                    }
                }
                
                tubeMesh.draw();
            }
            material.end();
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
    // Calculate new dimensions with bounds checking
    int newWidth = std::clamp((ofGetWidth() / cellSize - 1) / 2, 5, 100);
    int newHeight = std::clamp((ofGetHeight() / cellSize - 1) / 2, 5, 100);
    
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
