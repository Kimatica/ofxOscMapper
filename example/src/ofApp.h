#pragma once

#include "ofMain.h"
#include "ofxPanel.h"
#include "ofxOscMapper.h"


class ofApp : public ofBaseApp{

public:
    
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    
    ofxOscMapper osc;
    
    ofParameter<float> size;
    ofParameter<int> number;
    ofParameter<bool> check;
    ofParameter<ofColor> color;
    ofParameterGroup parameters;
    ofParameterGroup parameters3;
    ofxPanel gui;
    
    ofParameter<float> size2;
    ofParameter<int> number2;
    ofParameter<bool> check2;
    ofParameter<ofColor> color2;
    ofParameterGroup parameters2;
    ofxPanel gui2;
    
    bool bDrawGui;
};
