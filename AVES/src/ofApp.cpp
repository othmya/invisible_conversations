#include "ofApp.h"
#include "ofxJSON.h"
#include "ofxDatGui.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>

std::vector<ofColor> availableColors = {
    ofColor(255, 0, 0),    // Red
    ofColor(0, 255, 0),    // Green
    ofColor(0, 0, 255),    // Blue
    ofColor(255, 255, 0),  // Yellow
    ofColor(255, 165, 0),  // Orange
    ofColor(128, 0, 128),  // Purple
    ofColor(0, 255, 255),  // Cyan
    ofColor(255, 192, 203)  // Pink
};


// Function to count unique landuse categories
int countUniqueLanduseCategories(const std::vector<std::string>& landuseCategories) {
    std::unordered_set<std::string> uniqueCategories(landuseCategories.begin(), landuseCategories.end());
    return uniqueCategories.size();
}

// Function to generate color for landuse based on index
ofColor generateColorForLanduse(int index, int maxCategories) {
    // Generate a color based on the index and the number of unique categories
    float hue = ofMap(index, 0, maxCategories - 1, 0, 255); // Map index to hue
    return ofColor::fromHsb(hue, 255, 255); // HSB color space
}

ofColor generateColorForTime(int timeValue) {
    // Map timeValue (HHMM) to a color
    float normalizedTime = ofMap(timeValue, 0, 2359, 0, 1); // Normalize to [0, 1]
    return ofColor::fromHsb(normalizedTime * 255, 255, 255); // HSB color space
}

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

                    // Extract and store the path (This is where the MP3 file is stored)
                    std::string path = json[i].isMember("path") ? json[i]["path"].asString() : "";
                    // size_t pos = path.find_last_of('/');
                    // if (pos != std::string::npos) {
                    //     path = path.substr(pos + 1);
                    // }
                    paths.push_back(path); // Store the processed path

                    // Extract and store the species name
                    std::string species = json[i].isMember("sp") ? json[i]["sp"].asString() : "";
                    speciesNames.push_back(species);

                    // Extract and store the remark
                    std::string remark = json[i].isMember("remark") ? json[i]["remark"].asString() : "";
                    remarks.push_back(remark);

                    pointIntersected.push_back(false);

                    // Extract and store the landuse category
                    std::string landuse = json[i].isMember("landuse") ? json[i]["landuse"].asString() : "";
                    landuseCategories.push_back(landuse);

                    // // Assign a color if this landuse category is new
                    // if (landuseColorMap.find(landuse) == landuseColorMap.end()) {
                    //     ofColor color = generateColor(landuseColorMap.size());
                    //     landuseColorMap[landuse] = color;
                    // }

                    // Extract and store the time variable
                    std::string timeStr = json[i].isMember("time") ? json[i]["time"].asString() : "";
                    if (!timeStr.empty()) {
                        // Transform HH:MM to HHMM
                        size_t colonPos = timeStr.find(':');
                        if (colonPos != std::string::npos) {
                            int hours = std::stoi(timeStr.substr(0, colonPos));
                            int minutes = std::stoi(timeStr.substr(colonPos + 1));
                            int timeValue = hours * 100 + minutes; // Convert to HHMM format
                            timeValues.push_back(timeValue);
                        }
                    }

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
    cam.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, 1, 0)); // Look at the center
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
    // instructionText = "Use WASD to rotate the point cloud";

    // Font loading
    font.load("verdana.ttf", 20);

    rotationX = 0;
    rotationY = 0;
    rotateLeft = rotateRight = rotateUp = rotateDown = false;

    soundPlayer.setMultiPlay(true);  // Allows overlapping sound if accessed quickly
    soundPlayer.setLoop(false);  

    // Initialize previousColors with the same size as points
    previousColors.resize(points.size(), ofColor(0, 0, 0, 0)); // Initialize with transparent black

    // Add a dropdown for color selection
    auto colorModeDropdown = gui->addDropdown("Color Mode", {"Landuse", "Time"});
    colorModeDropdown->onDropdownEvent(this, &ofApp::onDropdownEvent);

    // Add a slider for distance threshold
    distanceThreshold = 1.0f; // Set an initial value for the distance threshold

    // Create a slider for distance threshold
    distanceThresholdSlider = gui->addSlider("Distance Threshold", 1, 100, distanceThreshold);
    distanceThresholdSlider->onSliderEvent(this, &ofApp::onSliderEvent); // Attach event handler
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

        // thalia additions
        soundPlayer.load(paths[currentNodeIndex]);
        soundPlayer.play();
        
        // Add to visited nodes
        if (std::find(visitedNodes.begin(), visitedNodes.end(), currentNodeIndex) == visitedNodes.end()) {
            visitedNodes.push_back(currentNodeIndex);
        }

        if (isCrossfading) {
            // Calculate the elapsed time since the crossfade started
            float elapsedTime = ofGetElapsedTimef() - crossfadeStartTime;
            float t = ofMap(elapsedTime, 0, crossfadeDuration, 0, 1, true); // Normalize to [0, 1]

            // Update volumes based on the elapsed time
            currentVolume = 1.0f - t; // Fade out current sound
            nextVolume = t; // Fade in next sound

            // Set the volumes
            currentSoundPlayer.setVolume(currentVolume);
            nextSoundPlayer.setVolume(nextVolume);

            // Check if the crossfade is complete
            if (elapsedTime >= crossfadeDuration) {
                isCrossfading = false; // End crossfade
                currentSoundPlayer.stop(); // Stop the current sound
                currentSoundPlayer = nextSoundPlayer; // Move next sound to current
            }
        }

        // Set the trail start time when a new node is visited
        trailStartTime = ofGetElapsedTimef(); // Record the current time
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

    // In the draw function, modify the point drawing logic
    int maxLanduseCategories = countUniqueLanduseCategories(landuseCategories); // Get the count of unique categories

    // Draw the point cloud (only once)
    ofMesh pointCloud;
    pointCloud.setMode(OF_PRIMITIVE_POINTS);
    // In the draw function, modify the point drawing logic
    for (size_t i = 0; i < points.size(); ++i) {
        pointCloud.addVertex(points[i]);

        // Set the default color to black
        ofColor currentColor = ofColor(0, 0, 0); // Default to black

        // Check if the point is nearby using the distance threshold
        if (currentPosition.distance(points[i]) < distanceThreshold) {
            // Color based on the selected mode
            if (colorMode == "Landuse") {
                // Generate color based on the index of the landuse category
            int landuseIndex = std::distance(landuseCategories.begin(), std::find(landuseCategories.begin(), landuseCategories.end(), landuseCategories[i]));
            currentColor = generateColorForLanduse(landuseIndex, maxLanduseCategories); // Pass the max categories
            } else if (colorMode == "Time") {
                // Get the color based on time
                if (i < timeValues.size()) {
                    currentColor = generateColorForTime(timeValues[i]);
                }
            }

        }

        // Interpolate between the previous color and the current color
        ofColor interpolatedColor = previousColors[i].getLerped(currentColor, 0.1); // 0.1 is the interpolation factor
        pointCloud.addColor(interpolatedColor);

        // Update the previous color to the current color for the next frame
        previousColors[i] = currentColor;
    }
    
    glPointSize(sphereSize * ofGetWidth() / 500.0); // Point size based on screen width
    pointCloud.draw();

    // Draw the path of the random walk
    if (ofGetElapsedTimef() - trailStartTime < 15.0f) {
        ofSetColor(255, 255, 255, 50); // Change color to white with some transparency
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

    // Draw landuse categories on the left side of the window
    ofSetColor(255, 255, 255);
    float yOffset = 30; // Starting y position
    for (const auto& category : landuseColorMap) {
        ofSetColor(category.second);
        font.drawString(category.first, 20, yOffset);
        yOffset += 20; // Increment y position for the next category
    }

    // Draw the GUI last to ensure it's on top
    gui->draw();
}

//--------------------------------------------------------------
void ofApp::onSliderEvent(ofxDatGuiSliderEvent e) {
    if (e.target == distanceThresholdSlider) {
        distanceThreshold = e.value; // Update the distance threshold
        ofLog() << "Distance Threshold updated to: " << distanceThreshold; // Log for debugging
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
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e) {
    colorMode = e.target->getLabel(); // Store the selected color mode
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