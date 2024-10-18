#pragma once
#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxDatGui.h"
#include "ofSoundPlayer.h" 
#include <unordered_map>
#include <vector>

class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();

    void keyPressed(int key);
    void keyReleased(int key);
    // void mouseDragged(ofMouseEventArgs &mouse);
    // void mousePressed(int x, int y, int button);
    // void mouseReleased(int x, int y, int button);

    ofEasyCam cam;
    ofPlanePrimitive plane;

    float rotationSpeed, rotationAngle;
    float rotationX, rotationY;
    bool rotateLeft, rotateRight, rotateUp, rotateDown;
    float sphereSize;
    float zoomFactor;
    float moveSpeed;

    vector<ofVec3f> points; // Stores the 3D points
    vector<std::string> paths; // Stores the full paths
    vector<std::string> speciesNames; // Stores the species names
    vector<std::string> remarks; // Stores the remarks for each point
    vector<bool> pointIntersected;

    vector<std::string> intersectingPaths; // Stores paths of intersecting points
    vector<std::string> intersectingSpecies; // Stores species names of intersecting points
    vector<std::string> intersectingRemarks; // Stores remarks of intersecting points

    struct Intersection {
        int index;
        float time;
    };
    
    vector<Intersection> recentIntersections;

    float intersectionDuration;

    // For the random walk generator
    ofVec3f currentPosition;
    int currentNodeIndex;
    float nextNodeDistanceProb;
    float stochasticityProb;
    float localWalkDistanceThreshold;

    // New variables for smooth transitions
    ofVec3f targetPosition;
    float transitionSpeed;

    // For sliders
    ofxDatGui* gui;
    ofxDatGuiSlider* nextNodeDistanceSlider;
    ofxDatGuiSlider* stochasticitySlider;
    
    void onSliderEvent(ofxDatGuiSliderEvent e);

    // Species name
    ofTrueTypeFont font;
    vector<int> visitedNodes;

    // Reset zoom button
    ofxDatGuiButton* resetZoomButton;
    void onButtonEvent(ofxDatGuiButtonEvent e);
    
    // Instruction text
    string instructionText;

    // Audio players for crossfade
    ofSoundPlayer currentSoundPlayer; // The currently playing sound
    ofSoundPlayer nextSoundPlayer; // The next sound to crossfade to
    float crossfadeDuration; // Duration of the crossfade in seconds
    float currentVolume; // Current volume of the current sound
    float nextVolume; // Current volume of the next sound
    bool isCrossfading; // Flag to indicate if a crossfade is in progress
    float crossfadeStartTime; // Start time for the crossfade

    // Shift pressed for initializing walk
    // bool shiftPressed;
    // additions thalia
    ofSoundPlayer soundPlayer;

    // New variables for landuse categories and colors
    std::vector<std::string> landuseCategories; // Stores landuse categories
    std::unordered_map<std::string, ofColor> landuseColorMap; // Map for landuse to color
    std::vector<ofColor> availableColors; // Vector of available colors

    // New variable for previous colors for crossfade effect
    std::vector<ofColor> previousColors; // To store the previous colors of the points

    // New variable for trail timing
    float trailStartTime; // To store the time when the trail starts

    // Add these lines to your ofApp class definition in ofApp.h
    std::vector<int> timeValues; // To store the transformed time values
    std::string colorMode; // To store the current color mode ("landuse" or "time")

    // Add the declaration for the dropdown event handler
    void onDropdownEvent(ofxDatGuiDropdownEvent e); // <-- Add this line

    // Another slider for coloring area
    ofxDatGuiSlider* distanceThresholdSlider; // Slider for distance threshold
    float distanceThreshold; // Variable to store the distance threshold value


    // Sound player
    std::vector<ofSoundPlayer> soundPlayers; // Vector to hold multiple sound players

    // Bias category and dropdown
    std::string biasCategory; // To store the selected bias category
    ofxDatGuiDropdown* biasDropdown; // Dropdown for bias selection
    ofxDatGuiDropdown* colorModeDropdown; // Dropdown for color mode selection
    ofxDatGuiDropdown* speciesDropdown; // Dropdown for species selection

    // Mouse click for starting the walk
    void mousePressed(int x, int y, int button);


};
