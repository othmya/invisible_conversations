#include "ofApp.h"
#include "ofxJSON.h"
#include "ofxDatGui.h"
// #include "ofxVideoRecorder.h"

//--------------------------------------------------------------
void ofApp::setup() {
    std::string jsonFilePath = "/home/oth/Downloads/OpenFrameworks/apps/myApps/AVES/bin/data/output_all_v1.json"; 

    ofxJSONElement json;
    bool success = json.open(jsonFilePath);

    if (success) {
        for (Json::ArrayIndex i = 0; i < json.size(); ++i) {
            if (json[i].isMember("point") && json[i]["point"].isArray() && json[i]["point"].size() == 3) {
                try {
                    float x = json[i]["point"][0].asFloat();
                    float y = json[i]["point"][1].asFloat();
                    float z = json[i]["point"][2].asFloat();
                    points.push_back(ofVec3f(x, y, z));

                    // Extract and store the path
                    std::string path = json[i].isMember("path") ? json[i]["path"].asString() : "";
                    size_t pos = path.find_last_of('/');
                    if (pos != std::string::npos) {
                        path = path.substr(pos + 1);
                    }
                    paths.push_back(path); // Store the processed path

                    // Extract and store the species name
                    std::string species = json[i].isMember("sp") ? json[i]["sp"].asString() : "";
                    speciesNames.push_back(species);

                    // Extract and store the remark
                    std::string remark = json[i].isMember("remark") ? json[i]["remark"].asString() : "";
                    remarks.push_back(remark);

                    pointIntersected.push_back(false);
                } catch (const std::exception& e) {
                    ofLogError("JSON Parsing Error") << "Error parsing point at index " << i << ": " << e.what();
                }
            } else {
                ofLogError("JSON Structure Error") << "Invalid or missing 'point' data at index " << i;
            }
        }
    } else {
        ofLogError("JSON Load Error") << "JSON file could not be loaded from " + jsonFilePath;
    }

    rotationSpeed = 0.0005; // Slow down rotation
    rotationAngle = 0.0;
    sphereSize = 2;
    zoomFactor = 1.0;
    moveSpeed = 0.5f;  // Slow down the movement speed for more control
    ofSetFrameRate(60);
    ofBackground(0, 0, 0); // Set background color to black

    // Set up the camera to start closer to the point cloud
    cam.setDistance(100); // Adjust this value to set the initial distance
    cam.setPosition(0, 0, 100); // Set an initial position
    cam.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, 1, 0)); // Look at the centerç
    // cam.disableMouseInput();


    currentNodeIndex = ofRandom(points.size());
    currentPosition = points[currentNodeIndex];
    nextNodeDistanceProb = 0.7; // 70% chance of choosing a closer node

    targetPosition = currentPosition;
    transitionSpeed = 0.05f; // Adjust this value to control the speed of movement

    // Sliders
    gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
    
    nextNodeDistanceSlider = gui->addSlider("Next Node Distance", 0, 1, nextNodeDistanceProb);
    resetZoomButton = gui->addButton("Reset Zoom");
    
    gui->onSliderEvent(this, &ofApp::onSliderEvent);
    gui->onButtonEvent(this, &ofApp::onButtonEvent);
    
    gui->setAutoDraw(false);


    // Customize the slider and button
    nextNodeDistanceSlider->setWidth(300, 0.3);
    resetZoomButton->setWidth(300);
    nextNodeDistanceSlider->setLabelAlignment(ofxDatGuiAlignment::LEFT);


    // Set the GUI theme to improve visibility
    gui->setTheme(new ofxDatGuiThemeSmoke());

    // Adjust the position of the GUI
    gui->setPosition(ofGetWidth() - 320, 20);

    // Set up instruction text
    instructionText = "Use WASD to rotate the point cloud";

    // Font loading
    font.load("verdana.ttf", 20);

    rotationX = 0;
    rotationY = 0;
    rotateLeft = rotateRight = rotateUp = rotateDown = false;
}

//--------------------------------------------------------------
void ofApp::update() {
    float currentTime = ofGetElapsedTimef();

    // Apply rotation based on key presses
    float rotationSpeed = 1.0; // Adjust this value to change rotation speed
    if (rotateLeft) rotationY -= rotationSpeed;
    if (rotateRight) rotationY += rotationSpeed;
    if (rotateUp) rotationX -= rotationSpeed;
    if (rotateDown) rotationX += rotationSpeed;

    // Smoothly move towards the target position
    currentPosition = currentPosition.getInterpolated(targetPosition, transitionSpeed);

    // Only choose a new target when we're close to the current target
    if (currentPosition.distance(targetPosition) < 0.1) {
        // Find next node based on distance probability
        vector<pair<int, float>> distances;
        for (int i = 0; i < points.size(); ++i) {
            if (i != currentNodeIndex) {
                float dist = currentPosition.distance(points[i]);
                distances.push_back(make_pair(i, dist));
            }
        }
        
        sort(distances.begin(), distances.end(), 
             [](const pair<int, float>& a, const pair<int, float>& b) {
                 return a.second < b.second;
             });
        
        int nextNodeIndex;
        if (ofRandom(1.0) < nextNodeDistanceProb) {
            // Choose a closer node
            nextNodeIndex = distances[ofRandom(distances.size() / 2)].first;
        } else {
            // Choose a farther node
            nextNodeIndex = distances[ofRandom(distances.size() / 2, distances.size())].first;
        }
        
        currentNodeIndex = nextNodeIndex;
        targetPosition = points[currentNodeIndex];

        // Add to visited nodes
        if (std::find(visitedNodes.begin(), visitedNodes.end(), currentNodeIndex) == visitedNodes.end()) {
            visitedNodes.push_back(currentNodeIndex);
        }
    }

    gui->update();
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofEnableDepthTest();

    // Draw instruction text
    ofSetColor(255, 255, 255);
    font.drawString(instructionText, 20, 30);

    cam.begin();
    
    ofPushMatrix();
    ofRotateXDeg(rotationX);
    ofRotateYDeg(rotationY);

    // Draw the point cloud (only once)
    ofMesh pointCloud;
    pointCloud.setMode(OF_PRIMITIVE_POINTS);
    for (size_t i = 0; i < points.size(); ++i) {
        pointCloud.addVertex(points[i]);
        if (i == currentNodeIndex) {
            pointCloud.addColor(ofColor(255, 0, 0, 255)); // Red for current node
        } else if (std::find(visitedNodes.begin(), visitedNodes.end(), i) != visitedNodes.end()) {
            pointCloud.addColor(ofColor(0, 255, 0, 255)); // Red for visited nodes
        } else if (pointIntersected[i]) {
            pointCloud.addColor(ofColor(46, 22, 77, 255)); // Purple for intersecting points
        } else {
            pointCloud.addColor(ofColor(255, 255, 255, 255)); // White for non-intersecting points
        }
    }
    
    glPointSize(sphereSize * ofGetWidth() / 1000.0);
    pointCloud.draw();

    // Draw the path of the random walk
    if (visitedNodes.size() > 1) {
        ofSetColor(209, 174, 235, 150); // Yellow with some transparency
        ofSetLineWidth(2);
        ofNoFill();
        ofBeginShape();
        for (const auto& nodeIndex : visitedNodes) {
            ofVertex(points[nodeIndex]);
        }
        ofEndShape(false);
    }

    // Draw connections from current node to nearby nodes
    ofSetColor(191, 189, 217, 50); // Purple for connections
    for (size_t i = 0; i < points.size(); ++i) {
        if (i != currentNodeIndex && currentPosition.distance(points[i]) < 10.0) {
            ofDrawLine(currentPosition, points[i]);
        }
    }

    ofPopMatrix();

    cam.end();

    ofDisableDepthTest();

    // Draw species name at the bottom center
    string speciesName = speciesNames[currentNodeIndex];
    ofRectangle bounds = font.getStringBoundingBox(speciesName, 0, 0);
    float x = (ofGetWidth() - bounds.width) / 2;
    float y = ofGetHeight() - bounds.height - 20;
    
    ofSetColor(0, 0, 0);
    ofDrawRectangle(x - 10, y - 10, bounds.width + 20, bounds.height + 20);
    ofSetColor(255, 255, 255);
    font.drawString(speciesName, x, y + bounds.height);

    // Draw the GUI last to ensure it's on top
    gui->draw();
}

//--------------------------------------------------------------
void ofApp::onSliderEvent(ofxDatGuiSliderEvent e) {
    if (e.target == nextNodeDistanceSlider) {
        nextNodeDistanceProb = e.value;
    }
}

//--------------------------------------------------------------

void ofApp::onButtonEvent(ofxDatGuiButtonEvent e) {
    if (e.target == resetZoomButton) {
        cam.reset();
        cam.setDistance(100); // Set to your desired default distance
    }
}


//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 'a') rotateLeft = true;
    if (key == 'd') rotateRight = true;
    if (key == 'w') rotateUp = true;
    if (key == 's') rotateDown = true;
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
    if (key == 'a') rotateLeft = false;
    if (key == 'd') rotateRight = false;
    if (key == 'w') rotateUp = false;
    if (key == 's') rotateDown = false;
}


// //--------------------------------------------------------------
// void ofApp::startRecording() {
//     if (!isRecording) {
//         // Setup video recorder
//         videoRecorder.setup(videoFilePath, ofGetWidth(), ofGetHeight(), 30); // Check if frame rate is supported
//         isRecording = true;
//         frameCount = 0;
//         ofLogNotice() << "Recording started";
//     }
// }

// //--------------------------------------------------------------
// void ofApp::stopRecording() {
//     if (isRecording) {
//         videoRecorder.close(); // Finalize and close the video file
//         isRecording = false;
//         ofLogNotice() << "Recording stopped";
//     }
// }
