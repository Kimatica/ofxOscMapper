#include "ofApp.h"


void ofApp::setup(){
    ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetVerticalSync(true);
    ofBackground(10);
    
    //create some guis
    
    parameters.setName("parameters 1");
    parameters.add(size.set("size",10,1,500));
    parameters.add(number.set("number",100,1,500));
    parameters.add(check.set("check",false));
    parameters.add(color.set("color",ofColor(127),ofColor(0,0),ofColor(255)));
    // test parameterGroup nesting
    parameters3.setName("parameters 3");
    parameters3.add(parameters);
    gui.setup("panel nested groups");
    gui.add(parameters3);
    
    parameters2.setName("parameters 2");
    parameters2.add(size2.set("size",10,1,100));
    parameters2.add(number2.set("number",100,1,100));
    parameters2.add(check2.set("check",false));
    parameters2.add(color2.set("color",ofColor(127),ofColor(0,0),ofColor(255)));
    gui2.setup(parameters2);
    gui2.setPosition(250, 10);
    
    // start oscMapper setting incomming port
    osc.setup(8001);
    // add the panels you want to map
    osc.addPanel(&gui);
    osc.addPanel(&gui2);
    // load previously saved mapping
    osc.loadOscMapping();
    
    bDrawGui = true;
}


void ofApp::update(){
}


void ofApp::draw(){
    ofPushStyle();
    ofSetColor(color);
    for(int i=0;i<number;i++){
        ofCircle(ofGetWidth()*.5-size*((number-1)*0.5-i), ofGetHeight()*.5, size);
    }
    ofPopStyle();
    
    if(bDrawGui){
        gui.draw();
        gui2.draw();
    }
    
    ofPushMatrix();
    ofTranslate(0, ofGetHeight() - 100);
    ofDrawBitmapString("toggle mapping mode (m)", 20, 0);
    ofDrawBitmapString("save osc mapping (s)", 20, 20);
    ofDrawBitmapString("enable/disable osc (x)", 20, 40);
    ofDrawBitmapString("toggle gui (g)", 20, 60);
    ofDrawBitmapString("toggle fullscreen (f)", 20, 80);
    ofPopMatrix();
}


void ofApp::keyPressed(int key){
    switch (key) {
        case 'f':
            ofToggleFullscreen();
            break;
        case 'm':
            osc.toggleMappingMode();
            break;
        case 'g':
            bDrawGui = !bDrawGui;
            break;
        case 's':
            osc.saveOscMapping();
            break;
        case 'x':
            if(osc.isEnabled())
                osc.disable();
            else
                osc.enable();
            break;
        default:
            break;
    }
}
