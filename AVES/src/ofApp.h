#pragma once

#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxDatGui.h"
#include "ofSoundPlayer.h" 
#include <unordered_map>
#include <vector>

// Main application class
class ofApp : public ofBaseApp {
public:
    // Setup, update, and draw methods
    void setup();
    void update();
    void draw();

    // Key and mouse event handlers
    void keyPressed(int key);
    void keyReleased(int key);
    // void mousePressed(int x, int y, int button);
    void mouseMoved(int x, int y); // Declare mouseMoved function


    // Camera and movement variables
    ofEasyCam cam;
    float sphereSize;
    float zoomFactor;
    float moveSpeed;
    float cameraDistance;
    // Data storage
    std::vector<ofVec3f> points; // Stores the 3D points
    std::vector<std::string> paths; // Stores the full paths
    std::vector<std::string> speciesNames; // Stores the species names
    std::vector<std::string> remarks; // Stores the remarks for each point
    std::vector<std::string> landuseCategories; // Stores landuse categories
    std::vector<int> timeValues; // To store the transformed time values

    // Original data storage
    std::vector<ofVec3f> originalPoints; // Store original points
    std::vector<std::string> originalPaths; // Store original paths
    std::vector<std::string> originalSpeciesNames; // Store original species names
    std::vector<std::string> originalRemarks; // Store original remarks
    std::vector<std::string> originalLanduseCategories; // Store original landuse categories
    std::vector<int> originalTimeValues; // Store original time values

    // Intersection structure for recent intersections
    struct Intersection {
        int index;
        float time;
    };
    
    std::vector<Intersection> recentIntersections;

    // Timing and movement variables
    float intersectionDuration;
    ofVec3f currentPosition;
    int currentNodeIndex;
    float nextNodeDistanceProb;
    float localWalkDistanceThreshold;
    ofVec3f targetPosition; // For smooth transitions
    float transitionSpeed;

    // GUI elements
    ofxDatGui* gui; // GUI manager
    ofxDatGuiButton* resetZoomButton; // Button to reset zoom
    ofxDatGuiButton* trackPlayheadButton; // Button to track the playhead
    bool trackPlayhead; // Boolean to track the playhead
    float cameraAngle; // Distance from the camera to the current position
    void onButtonEvent(ofxDatGuiButtonEvent e); // Button event handler
    ofxDatGuiSlider* transitionSpeedSlider; // Slider for transition speed

    void setupGui();
    void drawGui(ofEventArgs & args);
    // Slider event handler
    void onSliderEvent(ofxDatGuiSliderEvent e);

    // Font and instruction text
    ofTrueTypeFont font;
    std::vector<int> visitedNodes;    
    std::string instructionText;

    // Landuse color mapping
    std::unordered_map<std::string, ofColor> landuseColorMap; // Map for landuse to color
    std::vector<ofColor> availableColors; // Vector of available colors
    std::vector<ofColor> previousColors; // To store the previous colors of the points

    // Trail timing
    float trailStartTime; // To store the time when the trail starts

    // Color mode
    std::string colorMode; // To store the current color mode ("landuse" or "time")

    // Dropdown event handler
    void onDropdownEvent(ofxDatGuiDropdownEvent e); 

    // Distance threshold slider
    ofxDatGuiSlider* distanceThresholdSlider; // Slider for distance threshold
    float distanceThreshold; // Variable to store the distance threshold value

    // Local walk distance threshold slider
    ofxDatGuiSlider* localWalkDistanceThresholdSlider; // Slider for local walk distance threshold

    // Bias category and dropdown
    std::string biasCategory; // To store the selected bias category
    ofxDatGuiDropdown* biasDropdown; // Dropdown for bias selection
    std::vector<std::string> biasOptions; // Vector for bias options
    std::unordered_map<std::string, std::string> landuseCategoryMap; // Map for landuse categories
    std::unordered_map<std::string, std::string> timeCategoryMap; // Map for time categories
    
    
    ofxDatGuiDropdown* colorModeDropdown;
    ofxDatGuiDropdown* cameraMovementDropdown; // Dropdown for camera movement selection
    std::string cameraMovement; // To store the current camera movement

    // Function to initialize the land use color map
    void initializeLanduseColorMap(const std::vector<std::string>& categories);
    ofColor getColorForLanduse(const std::string& category);

    bool guiVisible; // Variable to track GUI visibility

};
