#pragma once
#include "ofMain.h"
#include "ofxJSON.h"
#include "ofxDatGui.h"

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

    // Shift pressed for initializing walk
    // bool shiftPressed;
    // additions thalia
    ofSoundPlayer soundPlayer;

};