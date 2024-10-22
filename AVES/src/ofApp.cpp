#include "ofApp.h"
#include "ofxJSON.h"
#include "ofxDatGui.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <fstream>


// Function to count unique landuse categories
int countUniqueLanduseCategories(const std::vector<std::string>& landuseCategories) {
    std::unordered_set<std::string> uniqueCategories(landuseCategories.begin(), landuseCategories.end());
    return uniqueCategories.size();
}

void printUniqueLanduseCategories(const std::vector<std::string>& landuseCategories) {
    std::unordered_set<std::string> uniqueCategories(landuseCategories.begin(), landuseCategories.end());
    ofLog() << "Unique Land Use Categories:";
    for (const auto& category : uniqueCategories) {
        ofLog() << category; // Print each unique category
    }
}

ofColor generateColorForLanduse(const std::string& landuse) {
    // Define colors for different categories
    if (landuse == "forest" || landuse == "greenfield" || landuse == "orchard" || 
        landuse == "meadow" || landuse == "residential" || landuse == "village_green" || 
        landuse == "recreation_ground" || landuse == "greenery" || landuse == "farmland" || 
        landuse == "animal_enclosure" || landuse == "apiary" || landuse == "allotments" || 
        landuse == "greenhouse_horticulture" || landuse == "vineyard") {
        return ofColor(34, 139, 34); // Forest Green for green spaces
    } else if (landuse == "commercial" || landuse == "industrial" || landuse == "brownfield" || 
               landuse == "construction" || landuse == "cemetery" || landuse == "railway" || 
               landuse == "military" || landuse == "retail" || landuse == "landfill") {
        return ofColor(128, 128, 128); // Gray for gray spaces
    } else if (landuse == "reservoir" || landuse == "basin" || landuse == "quarry") {
        return ofColor(210, 180, 140); // Tan for mineral spaces
    } else if (landuse == "agricultural") {
        return ofColor(255, 255, 0); // Yellow for agricultural
    } else if (landuse == "wetland") {
        return ofColor(0, 255, 255); // Cyan for wetland
    } else if (landuse == "desert") {
        return ofColor(210, 180, 140); // Tan for desert
    }
    // Default to black if category not found
    return ofColor(255, 255, 255, 50);
}

ofColor generateColorForTime(int timeValue) {
    if (timeValue >= 600 && timeValue <= 1000) {
        return ofColor(255, 223, 186); // Early Morning (Light Orange)
    } else if (timeValue >= 1001 && timeValue <= 1200) {
        return ofColor(255, 255, 0); // Morning (Yellow)
    } else if (timeValue >= 1201 && timeValue <= 1800) {
        return ofColor(255, 165, 0); // Evening (Orange)
    } else if (timeValue >= 1801 && timeValue <= 2400) {
        return ofColor(0, 0, 255); // Night (Blue)
    } else if (timeValue >= 2401 || timeValue <= 559) {
        return ofColor(75, 0, 130); // Midnight (Indigo)
    }
    return ofColor(255, 255, 255, 50); // Default to black if time value is invalid
}

std::string ofApp::getTimeCategoryFromValue(int timeValue) {
    if (timeValue >= 600 && timeValue <= 1000) {
        return "early morning";
    } else if (timeValue >= 1001 && timeValue <= 1200) {
        return "morning";
    } else if (timeValue >= 1201 && timeValue <= 1800) {
        return "evening";
    } else if (timeValue >= 1801 && timeValue <= 2400) {
        return "night";
    } else if (timeValue >= 2401 || timeValue <= 559) {
        return "midnight";
    }
    return "unknown"; // Default category if time doesn't fit in any range
}

// Define new land use categories
std::unordered_map<std::string, std::string> landuseCategoryMap = {
    {"forest", "leafy"}, {"greenfield", "leafy"}, {"orchard", "leafy"},
    {"meadow", "leafy"}, {"residential", "leafy"}, {"village_green", "leafy"},
    {"recreation_ground", "leafy"}, {"greenery", "leafy"}, {"farmland", "leafy"},
    {"animal_enclosure", "leafy"}, {"apiary", "leafy"}, {"allotments", "leafy"},
    {"greenhouse_horticulture", "leafy"}, {"vineyard", "leafy"},
    {"commercial", "unexplored"}, {"industrial", "unexplored"}, {"brownfield", "unexplored"},
    {"construction", "unexplored"}, {"cemetery", "unexplored"}, {"railway", "unexplored"},
    {"military", "unexplored"}, {"retail", "unexplored"}, {"landfill", "unexplored"},
    {"reservoir", "water"}, {"basin", "water"}, {"quarry", "mineral"},
    {"agricultural", "leafy"}, {"wetland", "water"}, {"desert", "mineral"}
};

// Define new time categories
std::unordered_map<std::string, std::string> timeCategoryMap = {
    {"early morning", "early morning"}, {"morning", "morning"},
    {"evening", "evening"}, {"night", "night"}, {"midnight", "midnight"}
};

// Coloring by species
std::unordered_map<std::string, ofColor> speciesColorMap; // Map to store species and their corresponding colors

// In the setup function, after loading species names
std::unordered_set<std::string> uniqueSpecies; // Set to store unique species

// Create a vector for the new bias options
std::vector<std::string> biasOptions;

// Add this as a class member
int consecutiveBiasedMoves = 0;



//--------------------------------------------------------------
void ofApp::setup() {
    std::string jsonFilePath = "data/output_all.json"; 
    ofxJSONElement json;
    bool success = json.open(ofToDataPath("output_all.json"));

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


                    // Extract and store the landuse category
                    std::string landuse = json[i].isMember("landuse") ? json[i]["landuse"].asString() : "";
                    landuseCategories.push_back(landuse);

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

    sphereSize = 3;
    original_sphereSize = 3;
    // moveSpeed = 0.5f;  // Slow down the movement speed for more control
    ofSetFrameRate(60);
    ofBackground(0, 0, 0); // Set background color to black

    // Set up the camera to start closer to the point cloud
    cameraDistance = 30;
    cam.setDistance(cameraDistance); // Adjust this value to set the initial distance
    cam.setPosition(0, 0, cameraDistance); // Set an initial position
    cam.lookAt(ofVec3f(0, 0, 0), ofVec3f(0, 1, 0)); // Look at the center
    // Adjust the near and far clipping planes
    cam.setNearClip(0.01f); // Set the near clipping plane to 0.1 units
    cam.setFarClip(1000.0f); // Set the far clipping plane to 1000 units




    currentNodeIndex = ofRandom(points.size());
    currentPosition = points[currentNodeIndex];
    ofLog()<<"currentNodeIndexset "<<currentNodeIndex;
    targetPosition = currentPosition;

    // Add a slider for local walk distance threshold



    
    
    cameraAngle = 0.0f;

    gui->setAutoDraw(false);
    







    // Font loading
    font.load("/Users/thalia/Antoine/invisible_conversations/AVES/bin/data/ofxbraitsch/fonts/Verdana.ttf", 10);

    // Initialize previousColors with the same size as points
    previousColors.resize(points.size(), ofColor(0, 0, 0, 0)); // Initialize with transparent black




    // Add a slider for distance threshold



    // Set initial gui visibility to false


}



void ofApp::setupGui(){
    // setup
        landuseCategoryMap = {
        {"forest", "leafy"}, {"greenfield", "leafy"}, {"orchard", "leafy"},
        {"meadow", "leafy"}, {"residential", "leafy"}, {"village_green", "leafy"},
        {"recreation_ground", "leafy"}, {"greenery", "leafy"}, {"farmland", "leafy"},
        {"animal_enclosure", "leafy"}, {"apiary", "leafy"}, {"allotments", "leafy"},
        {"greenhouse_horticulture", "leafy"}, {"vineyard", "leafy"},
        {"commercial", "unexplored"}, {"industrial", "unexplored"}, {"brownfield", "unexplored"},
        {"construction", "unexplored"}, {"cemetery", "unexplored"}, {"railway", "unexplored"},
        {"military", "unexplored"}, {"retail", "unexplored"}, {"landfill", "unexplored"},
        {"reservoir", "water"}, {"basin", "water"}, {"quarry", "mineral"},
        {"agricultural", "leafy"}, {"wetland", "water"}, {"desert", "mineral"}
    };

    timeCategoryMap = {
        {"early morning", "early morning"}, {"morning", "morning"},
        {"evening", "evening"}, {"night", "night"}, {"midnight", "midnight"}
    };



    // Set up the GUI
    gui = new ofxDatGui(ofxDatGuiAnchor::TOP_RIGHT);
    gui->setTheme(new ofxDatGuiThemeSmoke());
    gui->setWidth(800); // Set the desired width for the GUI
    gui->onSliderEvent(this, &ofApp::onSliderEvent);
    gui->onButtonEvent(this, &ofApp::onButtonEvent);

    localWalkDistanceThreshold = 8.0f; // Set an initial value for the local walk distance threshold
    localWalkDistanceThresholdSlider = gui->addSlider("Local Walk Distance Threshold", 2, 50, localWalkDistanceThreshold);
    localWalkDistanceThresholdSlider->onSliderEvent(this, &ofApp::onSliderEvent); // Attach event handler

    transitionSpeed = 0.02f; // Adjust this value to control the speed of movement
    transitionSpeedSlider = gui->addSlider("Transition Speed", 0.01f, 1.0f, transitionSpeed);
    transitionSpeedSlider->onSliderEvent(this, &ofApp::onSliderEvent); // Attach event handler

    resetZoomButton = gui->addButton("Reset Zoom");
    resetZoomButton->setWidth(500);

        // Set the GUI theme to improve visibility
    gui->setTheme(new ofxDatGuiThemeCharcoal());

    // Adjust the position of the GUI
    gui->setPosition(ofGetWidth() - 320, 20);

    distanceThreshold = 5.0f; // Set an initial value for the distance threshold
    distanceThresholdSlider = gui->addSlider("Distance Threshold", 2, 22, distanceThreshold);
    distanceThresholdSlider->onSliderEvent(this, &ofApp::onSliderEvent); // Attach event handler

        // Clear previous options
    biasOptions.clear();

    // Use a set to collect unique bias options
    std::unordered_set<std::string> uniqueBiasOptions;
    std::cout<<"hiiii "<<(landuseCategoryMap.size());
    // Add land use categories to the bias options
    for (const auto& pair : landuseCategoryMap) {
        uniqueBiasOptions.insert(pair.second); // Add the broader category (e.g., "leafy", "mineral", etc.)
    }

    // Add time categories to the bias options
    for (const auto& pair : timeCategoryMap) {
        uniqueBiasOptions.insert(pair.first); // Add the time category (e.g., "early morning", "morning", etc.)
    }

    // Populate the biasOptions vector with unique options
    biasOptions.assign(uniqueBiasOptions.begin(), uniqueBiasOptions.end());
    

    biasDropdown = gui->addDropdown("Bias Towards", biasOptions); // Add dropdown to the GUI
    biasDropdown->setPosition(10, 10); // Set position of the dropdown
    biasDropdown->onDropdownEvent(this, &ofApp::onDropdownEvent); // Attach event handler

    // Set default bias category
    biasCategory = "leafy"; // Default bias category

 

    for (const auto& species : speciesNames) {
        uniqueSpecies.insert(species); // Add species to the set
    }

    // Generate random colors for each unique species
    for (const auto& species : uniqueSpecies) {
        float hue = ofRandom(0, 255); // Random hue
        float saturation = 200;
        float brightness = 200;
        ofColor color = ofColor::fromHsb(hue, saturation, brightness); // Create color from HSB
        speciesColorMap[species] = color; // Map species to color
    }
    

    // Add a dropdown for color selection
    colorModeDropdown = gui->addDropdown("Color Mode", {"Landuse", "Time", "Species"});
    colorModeDropdown->onDropdownEvent(this, &ofApp::onDropdownEvent);
    colorMode = "Species"; 
    ofLog() << "Initial color mode: " << colorMode;

    // Set up the camera movement dropdown
    cameraMovementDropdown = gui->addDropdown("Camera Movement", {"Circular", "Dolly In/Out", "Pan", "Tilt", "Track Playhead", "Default", "Spiral", "Oscillate Zoom", "Follow Path", "Wobble"});
    cameraMovementDropdown->onDropdownEvent(this, &ofApp::onDropdownEvent); // Attach event handler

    
}
//--------------------------------------------------------------
void ofApp::update() {
    float currentTime = ofGetElapsedTimef();

    // Smoothly move towards the target position
    currentPosition = currentPosition.getInterpolated(targetPosition, transitionSpeed);

    // Only choose a new target when we're close to the current target
    if (currentPosition.distance(targetPosition) < 0.001) {
        // Find next node based on distance threshold
        vector<int> nearbyNodes; // Vector to store nearby node indices
        vector<int> biasedNodes; // Vector to store nodes that match the bias category
        std::unordered_map<int, int> categoryCount; // Map to count nodes by category

        // Declare targetNodeIndex
        int targetNodeIndex = -1; // Initialize to an invalid index

        // Check if the bias category is valid
        bool isBiasCategoryValid = (landuseCategoryMap.find(biasCategory) != landuseCategoryMap.end()) ||
                                   (timeCategoryMap.find(biasCategory) != timeCategoryMap.end());

        // Define a smaller radius for stricter clustering check
        float smallerRadius = 8.0f; // Hardcoded smaller radius
        float currentRadius = localWalkDistanceThreshold; // Start with the original radius

        // First pass: Check for nearby nodes within the current radius
        for (int i = 0; i < points.size(); ++i) {
            if (i != currentNodeIndex) {
                float dist = currentPosition.distance(points[i]);
                if (dist < currentRadius) {
                    nearbyNodes.push_back(i);

                   
                    std::string category;
                    std::string broaderCategory;

                    if (colorMode == "Landuse") {
                        category = landuseCategories[i];
                        broaderCategory = landuseCategoryMap[category];
                    } else if (colorMode == "Time") {
                        category = std::to_string(timeValues[i]); // Convert time value to string
                        broaderCategory = getTimeCategoryFromValue(timeValues[i]); // New function to get time category
                    } else if (colorMode == "Species") {
                        category = speciesNames[i];
                        broaderCategory = category; // For species, we use the species name directly
                    }
    
                    // ofLog() << "Category: " << category << ", Broader category: " << broaderCategory;

                    // Check if the current point's broader category matches the bias category
                    if (broaderCategory == biasCategory) {
                        biasedNodes.push_back(i);
                        categoryCount[i] += 1 + biasStrength;  // Add extra weight based on bias strength
                        ofLog() << "Biased node found: " << i;
                    }
                }
            }
        }

        // Check if more than 50% of nodes within the smaller radius are from the same category
        int biasedNodesInSmallerRadius = 0;
        int totalNodesInSmallerRadius = 0;

        for (int i = 0; i < points.size(); ++i) {
            if (i != currentNodeIndex) {
                float dist = currentPosition.distance(points[i]);
                if (dist < smallerRadius) {
                    totalNodesInSmallerRadius++;
                    if (std::find(biasedNodes.begin(), biasedNodes.end(), i) != biasedNodes.end()) {
                        biasedNodesInSmallerRadius++;
                    }
                }
            }
        }

        if (totalNodesInSmallerRadius > 0) {
            float percentage = (static_cast<float>(biasedNodesInSmallerRadius) / totalNodesInSmallerRadius) * 100.0f;
            if (percentage > 70.0f) {
                currentRadius = smallerRadius; // Reduce the radius to focus on the area
                ofLog() << "Biased nodes found in smaller radius, reducing radius to " << currentRadius;
            }
        }

        // Second pass: Check for clustering within the current radius (which might now be the smaller radius)
        if (!biasedNodes.empty()) {
            std::unordered_map<int, float> clusteringCount;
            float biasWeight = 10.0f;  // Increase this value to strengthen the bias
            for (const auto& node : biasedNodes) {
                for (int j = 0; j < points.size(); ++j) {
                    if (j != currentNodeIndex) {
                        float dist = points[node].distance(points[j]);
                        if (dist < currentRadius) {
                            clusteringCount[node] += (std::find(biasedNodes.begin(), biasedNodes.end(), j) != biasedNodes.end()) ? biasWeight : 1.0f;
                        }
                    }
                }
            }

            // Determine the target position based on the highest clustering count
            float maxCount = 0;
            for (const auto& node : biasedNodes) {
                if (clusteringCount[node] > maxCount) {
                    maxCount = clusteringCount[node];
                    targetNodeIndex = node;
                }
            }

            if (targetNodeIndex != -1) {
                targetPosition = points[targetNodeIndex]; // Set the new target position to the node with the highest clustering

                // Dynamically adjust the local walk distance threshold based on the distance to the target
                float distanceToTarget = currentPosition.distance(targetPosition);
                if (distanceToTarget > localWalkDistanceThreshold) {
                    localWalkDistanceThreshold = std::min(distanceToTarget * 1.5f, 30.0f); // Increase radius for long jumps
                }
            }
        } else if (!nearbyNodes.empty()) {
            if (ofRandom(1.0) < biasStrength) {
                // Choose from biased nodes if available, otherwise from all nearby nodes
                auto& choiceVector = !biasedNodes.empty() ? biasedNodes : nearbyNodes;
                targetPosition = points[choiceVector[ofRandom(choiceVector.size())]];
            } else {
                targetPosition = points[nearbyNodes[ofRandom(nearbyNodes.size())]];
            }
        }




        // Update the current node index to the target node index
        currentNodeIndex = targetNodeIndex; // Select the next node

        // After selecting the target node:
        if (targetNodeIndex != -1 && std::find(biasedNodes.begin(), biasedNodes.end(), targetNodeIndex) != biasedNodes.end()) {
            consecutiveBiasedMoves++;
            float momentumFactor = std::min(consecutiveBiasedMoves * 0.1f, 0.5f);
            localWalkDistanceThreshold *= (1.0f - momentumFactor);
            transitionSpeed *= (1.0f - momentumFactor);
        } else {
            consecutiveBiasedMoves = 0;
        }

        // Adjust the local walk distance threshold dynamically based on the clustering
        if (targetNodeIndex != -1) {
            localWalkDistanceThreshold = std::max(localWalkDistanceThreshold * 0.6f, 0.5f);  // More aggressive reduction
            transitionSpeed = std::max(transitionSpeed * 0.6f, 0.005f);  // More aggressive reduction
        } else {
            localWalkDistanceThreshold = std::min(localWalkDistanceThreshold * 1.5f, 25.0f);  // More aggressive increase
            transitionSpeed = std::min(transitionSpeed * 1.5f, 0.3f);  // More aggressive increase
        }
    }
            // playsounds
    std::vector<NodeWithDistance> nodesWithDistances;
    for (int i = 0; i < points.size(); ++i) {
        float dist = currentPosition.distance(points[i]);
        nodesWithDistances.push_back({i, dist});

    }
    std::sort(nodesWithDistances.begin(), nodesWithDistances.end(), [](const NodeWithDistance& a, const NodeWithDistance& b) {
    return a.distance < b.distance;
    });

    if (nodesWithDistances.size() > 6) {
            nodesWithDistances.resize(6);
    }
    // 
            // Play sounds for the closest 6 nodes (active + 5 nearest)
    for (int i = 0; i < nodesWithDistances.size(); ++i) {
        int nodeIndex = nodesWithDistances[i].index;
        ofVec3f nodePosition = points[nodeIndex];

        // If sound isn't already playing, start playing with fade-in effect
        if (soundPlayers.find(nodeIndex) == soundPlayers.end()) {
            ofSoundPlayer player;
            if (player.load(paths[nodeIndex])) {
                player.setMultiPlay(true);
                player.setLoop(false);
                player.play();
                player.setVolume(0); // Start with volume 0 for fade-in
                soundPlayers[nodeIndex] = player;
            }
        }

        // Fade-in or fade-out depending on distance
        float distance = currentPosition.distance(nodePosition);
        float targetVolume = ofMap(distance, 0, localWalkDistanceThreshold, 1.0, 0.0, true);
        float fadeDuration = 2.0f; // Duration for fade-in/out in seconds

        if (soundPlayers[nodeIndex].isPlaying()) {
            // Gradually adjust volume
            soundPlayers[nodeIndex].setVolume(ofLerp(soundPlayers[nodeIndex].getVolume(), targetVolume, fadeDuration * ofGetLastFrameTime()));
        }
    }

    if (cameraMovement == "circular") {
        float radius = 20; // Radius of the circular path
        float speed = 0.1; // Speed of the movement
        float angle = ofGetElapsedTimef() * speed; // Calculate the angle based on time

        cam.setPosition(radius * cos(angle), radius * sin(angle), cameraDistance);
        cam.lookAt(ofVec3f(0, 0, 0)); // Look at the center
    } else if (cameraMovement == "dolly") {
        static float distance = 100; // Initial distance
        static float speed = 0.1; // Speed of dolly movement
        static bool dollyIn = true; // Direction of movement

        if (dollyIn) {
            distance -= speed; // Move closer
            if (distance <= 5) dollyIn = false; // Change direction
        } else {
            distance += speed; // Move away
            if (distance >= 50) dollyIn = true; // Change direction
        }

        cam.setDistance(distance); // Set camera distance
        cam.lookAt(ofVec3f(0, 0, 0)); // Look at the center
    } else if (cameraMovement == "pan") {
        static float panSpeed = 1.5; // Speed of panning
        static float offsetX = 10; // Horizontal offset

        offsetX += panSpeed * ofGetLastFrameTime(); // Increment offset based on frame time
        cam.setPosition(offsetX, 0, cameraDistance); // Update camera position
        cam.lookAt(ofVec3f(0, 0, 0)); // Look at the center
    } else if (cameraMovement == "tilt") {
        static float tiltSpeed = 1.5; // Speed of tilting
        static float tiltAngle = 45; // Tilt angle

        tiltAngle += tiltSpeed * ofGetLastFrameTime(); // Increment angle based on frame time
        cam.setPosition(0, 0, cameraDistance); // Reset position
        cam.rotate(tiltAngle, 1, 0, 0); // Apply tilt rotation
        cam.lookAt(ofVec3f(0, 0, 0)); // Look at the center
    } else if (cameraMovement == "track") {
        trackPlayhead = true;
        sphereSize = 10;
        // Track playhead logic
        if (trackPlayhead) {
            cameraAngle += 0.001f; // Adjust the speed of the panning effect
            float radius = 2.0f; // Distance from the playhead
            float camX = currentPosition.x + radius * sin(cameraAngle);
            float camY = currentPosition.y + radius * cos(cameraAngle);
            float camZ = currentPosition.z; // Slightly above the playhead
            cam.setPosition(camX, camY, camZ); // Set the new camera position
            cam.lookAt(currentPosition); // Make the camera look at the current position
        }
    } else if (cameraMovement == "default") {
        // Restore default camera behavior
        cam.lookAt(ofVec3f(0, 0, 0)); // Look at the center
        trackPlayhead = false;
        sphereSize = original_sphereSize;
    } else if (cameraMovement == "spiral") {
        
        cam.reset(); // Restore camera to default position and orientation
        cam.setDistance(cameraDistance); // Set to your desired default distance
        trackPlayhead = false; // Reset tracking when camera is restored
        sphereSize = original_sphereSize; // Reset sphere size to the original value

        static float spiralAngle = 0; // Angle for the spiral movement
        static float spiralRadius = 20; // Initial radius
        static float spiralHeight = 0; // Height offset

        spiralAngle += 0.01; // Increment the angle more gently
        spiralRadius += 0.025; // Gradually increase the radius more gently
        spiralHeight += 0.05; // Gradually increase the height more gently

        float camX = spiralRadius * cos(spiralAngle);
        float camY = spiralHeight; // Use height for vertical movement
        float camZ = spiralRadius * sin(spiralAngle);

        cam.setPosition(camX, camY, camZ); // Set the new camera position
        cam.lookAt(ofVec3f(0, 0, 0)); // Look at the center

    } else if (cameraMovement == "oscillateZoom") {
        static float zoomSpeed = 0.5; // Speed of zooming
        static float zoomFactor = 0.5; // Current zoom factor

        zoomFactor += zoomSpeed * ofGetLastFrameTime(); // Increment zoom factor
        if (zoomFactor > 20 || zoomFactor < -20) {
            zoomSpeed *= -1; // Reverse direction when limits are reached
        }

        cam.setPosition(0, 0, cameraDistance + zoomFactor); // Adjust camera position based on zoom factor
        cam.lookAt(ofVec3f(0, 0, 0)); // Look at the center
    } else if (cameraMovement == "followPath") {
        static int pathIndex = 0; // Current index in the path
        static float speed = 0.5; // Speed of movement along the path

        // Ensure pathIndex is within bounds
        if (pathIndex < points.size()) {
            // Get the current and next points
            ofVec3f currentPoint = points[pathIndex];
            ofVec3f nextPoint = (pathIndex + 1 < points.size()) ? points[pathIndex + 1] : points[0]; // Loop back to the start

            // Interpolate between the current and next points
            float interpolationFactor = ofGetLastFrameTime() * speed; // Adjust speed based on frame time
            cam.setPosition(currentPoint * (1 - interpolationFactor) + nextPoint * interpolationFactor); // Smooth transition
            cam.lookAt(currentPoint); // Look at the current point

            // Increment pathIndex based on speed
            if (interpolationFactor >= 1.0) {
                pathIndex++; // Move to the next point
                if (pathIndex >= points.size()) {
                    pathIndex = 0; // Reset to the start of the path
                }
            }
        }
    } else if (cameraMovement == "wobble") {
        static float wobbleAmount = 0.5; // Amount of wobble
        static float wobbleSpeed = 1.0; // Speed of wobble

        float camX = cam.getPosition().x + wobbleAmount * sin(ofGetElapsedTimef() * wobbleSpeed);
        float camY = cam.getPosition().y + wobbleAmount * cos(ofGetElapsedTimef() * wobbleSpeed);
        float camZ = cam.getPosition().z;

        cam.setPosition(camX, camY, camZ); // Set the new camera position
        cam.lookAt(ofVec3f(0, 0, 0)); // Look at the center
    } else if (cameraMovement == "zoomOutAndPan") {
        float elapsedTime = ofGetElapsedTimef() - zoomOutAndPanStartTime;
        float duration = 10.0f; // Total duration of the effect in seconds
        float progress = ofClamp(elapsedTime / duration, 0, 1);

        // Zoom out effect
        float startDistance = 30;
        float endDistance = 100;
        float currentDistance = ofLerp(startDistance, endDistance, progress);

        // Panning effect
        float panAngle = progress * TWO_PI; // Full 360-degree pan
        float panRadius = 50;
        float camX = panRadius * cos(panAngle);
        float camZ = panRadius * sin(panAngle);

        // Set camera position
        cam.setPosition(camX, 0, camZ + currentDistance);
        cam.lookAt(ofVec3f(0, 0, 0));

        // Reset camera movement when effect is complete
        if (progress >= 1.0) {
            cameraMovement = "default";
            ofLog() << "Zoom out and pan effect completed";
        }
    }

    // Adjust sphere size based on camera distance
    float cameraDistance = cam.getDistance();
    // sphereSize = ofMap(cameraDistance, 0, 1000, 2, 10); // Map camera distance to sphere size (adjust values as needed)


    // gui->update();
}

//--------------------------------------------------------------
void ofApp::draw() {
    ofEnableDepthTest();

    // Draw instruction text
    ofSetColor(255, 255, 255);
    font.drawString(instructionText, 20, 30);

    cam.begin();
    
    ofPushMatrix();

    // In the draw function, modify the point drawing logic
    ofMesh pointCloud;
    pointCloud.setMode(OF_PRIMITIVE_POINTS);

    // Draw the point cloud (only once)
    for (size_t i = 0; i < points.size(); ++i) {
        pointCloud.addVertex(points[i]);

        // Calculate the distance from the current position
        float dist = currentPosition.distance(points[i]);

        // Determine the alpha value based on the distance
        float alpha;
        if (dist <= distanceThreshold) {
            alpha = 255; // Full opacity for points within the threshold
        } else if (dist >= 2.0 * distanceThreshold) {
            alpha = 0; // Fully transparent for points 5 times further away
        } else {
            // Interpolate alpha based on distance
            alpha = ofMap(dist, distanceThreshold, 2.0 * distanceThreshold, 255, 0, true);
        }

        // Set the color based on the selected color mode
        ofColor currentColor;
        if (colorMode == "Landuse") {
            currentColor = generateColorForLanduse(landuseCategories[i]); // Get the color based on landuse
        } else if (colorMode == "Time") {
            currentColor = generateColorForTime(timeValues[i]); // Get the color based on time
        } else if (colorMode == "Species") {
            currentColor = speciesColorMap[speciesNames[i]]; // Get the color based on species
        }
        currentColor.a = alpha; // Set the alpha value
        pointCloud.addColor(currentColor);
    }

    float new_sphereSize = sphereSize * ofGetWidth() / 1000.0;
    glPointSize(new_sphereSize); // Point size based on screen width
    pointCloud.draw();

    trailStartTime = ofGetElapsedTimef();

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
    // ofLog()<<"currentNodeIndex1 "<<currentNodeIndex;
    // Draw connections from current node to nearby nodes
    ofSetColor(191, 189, 217, trackPlayhead ? 10 : 30); // Purple for connections, alpha based on trackPlayhead
    for (size_t i = 0; i < points.size(); ++i) {
        if (i != currentNodeIndex && currentPosition.distance(points[i]) < 3.0 * distanceThreshold) {
            ofDrawLine(currentPosition, points[i]);
        }
    }

    ofPopMatrix();

    cam.end();

    ofDisableDepthTest();

    // Draw species name at the bottom center
    ofLog()<<"currentNodeIndex "<<currentNodeIndex;
    std::string speciesName = speciesNames[currentNodeIndex];
    ofLog()<<"speciesNamet "<<speciesName;
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

}


//--------------------------------------------------------------
void ofApp::drawGui(ofEventArgs & args){
	gui->draw();
}
//--------------------------------------------------------------
void ofApp::onSliderEvent(ofxDatGuiSliderEvent e) {
    if (e.target == distanceThresholdSlider) {
        distanceThreshold = e.value; // Update the distance threshold
    } else if (e.target == transitionSpeedSlider) {
        transitionSpeed = e.value; // Update the transition speed
    } else if (e.target == localWalkDistanceThresholdSlider) {
        localWalkDistanceThreshold = e.value; // Update the local walk distance threshold
    }
}

//--------------------------------------------------------------

void ofApp::onButtonEvent(ofxDatGuiButtonEvent e) {
    if (e.target == resetZoomButton) {
        cam.reset();
        cam.setDistance(cameraDistance); // Set to your desired default distance
        trackPlayhead = false; // Reset tracking when zoom is reset
        cameraMovement = "default"; // Reset camera movement to default
        sphereSize = original_sphereSize;
    } 
}


//--------------------------------------------------------------
void ofApp::onDropdownEvent(ofxDatGuiDropdownEvent e) {
    ofLog() << "Dropdown event triggered"; // Log to confirm the event is triggered
    if (e.target == colorModeDropdown) {
        colorMode = e.target->getLabel(); // Store the selected color mode
        ofLog() << "Color mode set to: " << colorMode; // Log for debugging
    } else if (e.target == biasDropdown) {
        biasCategory = e.target->getLabel(); // Update the bias category based on the selected option
        ofLog() << "Bias category set to: " << biasCategory; // Log for debugging
    } else if (e.target == cameraMovementDropdown) {
        // Handle camera movement selection
        std::string selectedMovement = e.target->getLabel();
        if (selectedMovement == "Circular") {
            cameraMovement = "circular";
        } else if (selectedMovement == "Dolly In/Out") {
            cameraMovement = "dolly";
        } else if (selectedMovement == "Pan") {
            cameraMovement = "pan";
        } else if (selectedMovement == "Tilt") {
            cameraMovement = "tilt";
        } else if (selectedMovement == "Track Playhead") {
            cameraMovement = "track"; // Set to track playhead
        } else if (selectedMovement == "Default") {
            cameraMovement = "default"; // Set to default behavior
            sphereSize = original_sphereSize;
        } else if (selectedMovement == "Spiral") {
            cameraMovement = "spiral"; // Set to spiral movement
        } else if (selectedMovement == "Oscillate Zoom") {
            cameraMovement = "oscillateZoom"; // Set to oscillate zoom movement
        } else if (selectedMovement == "Follow Path") {
            cameraMovement = "followPath"; // Set to follow path movement
        } else if (selectedMovement == "Wobble") {
            cameraMovement = "wobble"; // Set to wobble movement
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    if (key == 't' || key == 'T') {
        cameraMovement = "track";
        trackPlayhead = true;
        sphereSize = 10;        
        ofLog() << "Switched to Track Playhead mode";

    } else if (key == '1') {
        // Zoom in on the center of the point cloud
        ofVec3f center(0, 0, 0); // Assuming the center is at (0,0,0)
        float zoomDistance = 10; // Adjust this value to set how close you want to zoom
        colorMode = "Species";

        cam.setPosition(center);
        cam.move(0, 0, zoomDistance);
        cam.lookAt(center);

        distanceThreshold = 0; // Start value
        distanceThresholdSlider->setValue(distanceThreshold);
        // Update camera movement mode
        cameraMovement = "default";
        ofLog() << "Zoomed in on center, distance threshold set to 0";

    } else if (key == '2') {
        // Switch to "track" camera movement with reduced transition speed
        cameraMovement = "track";
        trackPlayhead = true;
        transitionSpeed = 0.02f;

        distanceThreshold = 8; // Start value
        distanceThresholdSlider->setValue(distanceThreshold);
        
        // Update the transition speed slider
        transitionSpeedSlider->setValue(transitionSpeed);
        
        ofLog() << "Switched to Track mode with reduced transition speed";

    } else if (key == '3') {
        cameraMovement = "circular";
        distanceThreshold = 10; // Start value
        distanceThresholdSlider->setValue(distanceThreshold);

        sphereSize = original_sphereSize; // Reset sphere size to the original value
        // Set initial camera position
        cam.setPosition(0, 0, 0); // Start closer to the center
        cam.lookAt(ofVec3f(0, 0, 0));

        ofLog() << "Started zoom out and pan effect";

    } else if (key == '4') {
        // Change color mode to Time
        colorMode = "Time";        
        ofLog() << "Color mode changed to Time";

        cameraMovement = "circular";

        localWalkDistanceThreshold = 15.0f; // Adjust this value as needed
        localWalkDistanceThresholdSlider->setValue(localWalkDistanceThreshold);
        
        // Increase transition speed
        transitionSpeed = 0.05f; // Adjust this value as needed
        transitionSpeedSlider->setValue(transitionSpeed);
        
        // Set bias to midnight
        biasCategory = "midnight";

    } else if (key == '5') {
        cam.reset();
        cam.setDistance(cameraDistance); // Set to your desired default distance
        trackPlayhead = false; // Reset tracking when zoom is reset
        cameraMovement = "default"; // Reset camera movement to default
        sphereSize = original_sphereSize;
        
        ofLog() << "Reset to default view. Spiral movement will start in 5 seconds.";
    } else if (key == '6') {
        cameraMovement = "spiral"; // Change camera movement to spiral
    } else if (key == 'z' || key == 'Z') {
        // Increase distance threshold by 0.05
        distanceThreshold += 0.1f;
        // Update the distance threshold slider
        distanceThresholdSlider->setValue(distanceThreshold);
        ofLog() << "Distance threshold increased to: " << distanceThreshold;
    } else if (key == 'x' || key == 'X') {
        // Decrease distance threshold by 0.05
        distanceThreshold -= 0.1f;
        // Update the distance threshold slider
        distanceThresholdSlider->setValue(distanceThreshold);
        ofLog() << "Distance threshold decreased to: " << distanceThreshold;
    } else if (key == 'c' || key == 'C') {
        // increase local walk distance threshold by 0.05
        localWalkDistanceThreshold += 0.5f;
        // Update the local walk distance threshold slider
        localWalkDistanceThresholdSlider->setValue(localWalkDistanceThreshold);
        ofLog() << "Local walk distance threshold increased to: " << localWalkDistanceThreshold;
    } else if (key == 'v' || key == 'V') {
        // decrease local walk distance threshold by 0.05
        localWalkDistanceThreshold -= 0.5f;
        // Update the local walk distance threshold slider
        localWalkDistanceThresholdSlider->setValue(localWalkDistanceThreshold);
        ofLog() << "Local walk distance threshold decreased to: " << localWalkDistanceThreshold;
    } else if (key == 'b' || key == 'B') {
        // increase transition speed by 0.05
        transitionSpeed += 0.1f;
        // Update the transition speed slider
        transitionSpeedSlider->setValue(transitionSpeed);
        ofLog() << "Transition speed increased to: " << transitionSpeed;
    } else if (key == 'n' || key == 'N') {
        // decrease transition speed by 0.05
        transitionSpeed -= 0.1f;
        // Update the transition speed slider
        transitionSpeedSlider->setValue(transitionSpeed);
        ofLog() << "Transition speed decreased to: " << transitionSpeed;
    } else if (key == 'q' || key == 'Q') {
        // change color to time
        colorMode = "Time";
        ofLog() << "Color mode changed to Time";
    } else if (key == 'w' || key == 'W') {
        // change color to species
        colorMode = "Species";
        ofLog() << "Color mode changed to Species";
    } else if (key == 'e' || key == 'E') {
        // change color to landuse
        colorMode = "Landuse";
        ofLog() << "Color mode changed to Landuse";
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key) {
}


//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y) {
    // // Check if the mouse is over the GUI area
    // if (gui->getPosition().x <= x && x <= gui->getPosition().x + gui->getWidth() &&
    //     gui->getPosition().y <= y && y <= gui->getPosition().y + gui->getHeight()) {
    //     guiVisible = true; // Show the GUI if mouse is over it
    // } else {
    //     guiVisible = false; // Hide the GUI if mouse is not over it
    // }
}