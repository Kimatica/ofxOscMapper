
#include "ofxOscMapper.h"


ofxOscMapper::ofxOscMapper(){
    bMapModeEnabled = false;
    bEnabled = false;
    selected = "";
    bParamenterSyncEnabled = false;
}


void ofxOscMapper::setup(int port){
    verdana14.loadFont("stan0755.ttf", 6, false, true);
    
    oscIn.setup(port);
    
    // TODO osc forwarding (aka osc echo)
    // output ports should be passed by parameter to setup
    // local echo to vezer
//    ofxOscSender* senderVezer = new ofxOscSender();
//    senderVezer->setup("localhost", 9000);
//    // remote echo to touchOsc
//    ofxOscSender* senderTouchOSC = new ofxOscSender();
//    senderTouchOSC->setup("169.254.39.22", 7000);
//    
//    oscOut.push_back(senderVezer);
//    oscOut.push_back(senderTouchOSC);
    
//    senderVezer.setup("localhost", 9000);
//    senderTouchOSC.setup("169.254.39.22", 7000);
    
    enable();
}



void ofxOscMapper::addPanel(ofxPanel* panel){
    addGroup(panel);
    panels.push_back(panel);
    
    // TODO: parameter sync
    // in OF 0.8.4 a group can only have one parent
    // see: https://github.com/openframeworks/openFrameworks/issues/3104
    //ofParameterGroup & group = (ofParameterGroup&) panel->getParameter();
    //cout << group.getParent() << endl;
    //ofAddListener(group.parameterChangedE, this, &ofxOscMapper::parameterChanged);
}


void ofxOscMapper::addGroup(ofxGuiGroup* group){
    for(int i = 0; i < group->getNumControls(); i++){
        ofxBaseGui* control = group->getControl(i);
        
        if(isGroup(control->getParameter())){
            
            ofxGuiGroup* group = dynamic_cast<ofxGuiGroup*>(control);
            addGroup(group);
            
        }else if(isMappeable(control->getParameter())){
            
            string controlName = control->getName();
            if(controlsNameCount.find(controlName) == controlsNameCount.end()) {
                // this name hasn't been added yet, init counter
                controlsNameCount[controlName] = 0;
            }else{
                // this name has already been added, increment counter
                controlsNameCount[controlName] += 1;
            }
            // get the count
            int nameCount = controlsNameCount[controlName];
            string uniqueName = controlName + "-" + ofToString(nameCount);
            
            // store control (avoiding duplicated keys)
            controls[uniqueName] = control;
            
            //cout << group->getName() << " >> " << uniqueName << " (" << control->getParameter().type() << ")" << endl;
        }
    }
}


void ofxOscMapper::update(ofEventArgs &args){
    // map mode
    if(bMapModeEnabled){
        while(oscIn.hasWaitingMessages()){
            address = "";
            ofxOscMessage message;
            oscIn.getNextMessage(&message);
            address = message.getAddress();
            
            // link
            if(selected != ""){
                
                //cout << "message type is " << message.getArgType(0) << endl;
                
                // check if param and osc types match
                if( matchTypes(selected, message) ){
                    
                    for(map<string,string>::iterator it=addressToName.begin(); it!=addressToName.end(); ++it){
                        // if the selected parameter is linked to an address
                        if(selected == it->second){
                            // erase the address linked to the selected parameter
                            addressToName.erase(it);
                            //stop searching
                            break;
                        }
                    }
                    
                    // create a link between the address and the parameter
                    // this will overide the value if the address aready exists
                    addressToName[address] = selected;
                }
                else {
                    //ofLogWarning("ofxOscMapper::update") << "[TYPE MISMATCH] " << selected << " <-> " << address;
                    
                    ofAbstractParameter& parameter = controls[selected]->getParameter();
                    vector<string> hierarchyNames = parameter.getGroupHierarchyNames();
                    std::string paramPath;
                    for (int i = 0; i < hierarchyNames.size(); i++) {
                        paramPath += "/" + hierarchyNames[i];
                    }
                    ofLogWarning("ofxOscMapper::update") << "[TYPE MISMATCH] " << paramPath << " <-> " << address;
                }
            }
        }
    }
    // play mode
    else{
        while(oscIn.hasWaitingMessages()){
            ofxOscMessage m;
            oscIn.getNextMessage(&m);
            address = m.getAddress();
            
            // if address is linked to a param...
            if(addressToName.find(address) != addressToName.end()){
                
                // TODO: echo the message
                // note that only messages mapped to a control are forwarded
//                for (int i = 0; i < oscOut.size(); i++) {
//                    oscOut[i]->sendMessage(m);
//                }
                
                string paramName = addressToName[address];
                ofAbstractParameter& parameter = controls[paramName]->getParameter();
                
                if(m.getNumArgs() == 3){
                    if(m.getArgType(0) == OFXOSC_TYPE_INT32){
                        parameter.cast<ofColor>() = ofColor(m.getArgAsInt32(0), m.getArgAsInt32(1), m.getArgAsInt32(2));
                    }
                    else if(m.getArgType(0) == OFXOSC_TYPE_FLOAT){
                        parameter.cast<ofFloatColor>() = ofFloatColor(m.getArgAsFloat(0), m.getArgAsFloat(1), m.getArgAsFloat(2));
                    }
                }
                else if (m.getNumArgs() == 1){
                    if(m.getArgType(0) == OFXOSC_TYPE_INT32){
                        parameter.cast<int>() = m.getArgAsInt32(0);
                    }
                    else if(m.getArgType(0) == OFXOSC_TYPE_FLOAT){
                        //ofParameter<float> & param = (ofParameter<float> &)parameters.get(paramName);
                        //param = m.getArgAsFloat(i);
                        parameter.cast<float>() = m.getArgAsFloat(0);
                        
                        ////////////////////////////////////////////////////////////////
                        // testing:
                        // float to bool (0.0 = false, 1.0 = true)
                        if(parameter.type() == typeid(ofParameter<bool>).name()){
                            parameter.cast<bool>() = m.getArgAsFloat(0);
                        }
                        // float to int
                        else if(parameter.type() == typeid(ofParameter<int>).name()){
                            parameter.cast<int>() = m.getArgAsFloat(0);
                        }
                        ////////////////////////////////////////////////////////////////
                    }
                    else if(m.getArgType(0) == OFXOSC_TYPE_STRING){
                        parameter.cast<string>() = m.getArgAsString(0);
                    }
                }
                else{
                    cout << "unknown argument type!" << endl;
                }
            }
        }
    }
}


void ofxOscMapper::draw(ofEventArgs &args){
    if(bMapModeEnabled){
        ofPushStyle();
        ofSetLineWidth(2);
        
        // draw the rectangles on top of the controls
        ofEnableBlendMode(OF_BLENDMODE_ALPHA);
        for(auto it = controls.begin(); it != controls.end(); ++it){
            string controlName = it->first;
            ofxBaseGui* control = it->second;
            
            if(controlName == selected){
                drawOverlay(control, colorHighlight, ofColor(255));
            }else{
                drawOverlay(control, colorNormal, ofColor(0));
            }
        }
        ofDisableBlendMode();
        
        // draw the labels of the mapped addresses
        for (map<string,string>::iterator it=addressToName.begin(); it!=addressToName.end(); ++it){
            string controlName = it->second;
            ofxBaseGui* control = controls[controlName];
            ofRectangle r = control->getShape();
            ofSetColor(255);
            ofRectangle bbox = verdana14.getStringBoundingBox(it->first, 0, 0);
            float x = fabs(r.getCenter().x - bbox.width / 2);
            float y = fabs(r.getCenter().y + bbox.height / 2);
            verdana14.drawString(it->first, x, y);
        }
        
        // draw mismatch message
        //ofDrawBitmapStringHighlight(mismatchMessage, 10, 10, ofColor::darkRed);
        
        ofPopStyle();
    }
}


void ofxOscMapper::mouseReleased(ofMouseEventArgs& mouse){
    selected = "";
    
    for(auto it = controls.begin(); it != controls.end(); ++it){
        string controlName = it->first;
        ofxBaseGui* control = it->second;
        
        if(control->getShape().inside(mouse.x, mouse.y)){
            selected = controlName;
        }
    }
}


void ofxOscMapper::keyPressed(ofKeyEventArgs& args){
    switch(args.key){
        case OF_KEY_BACKSPACE:
            // remove the link if the selected parameter is linked to an address
            for (map<string,string>::iterator it=addressToName.begin(); it!=addressToName.end(); ++it){
                string controlName = it->second;
                if(controlName == selected){
                    addressToName.erase(it);
                }
            }
            break;
        default:
            break;
    }
}


void ofxOscMapper::enableMappingMode(){
    if(!bEnabled || bMapModeEnabled){
        return;
    }
    
    bMapModeEnabled = true;
    
    for(int i = 0; i < panels.size(); ++i){
        panels[i]->unregisterMouseEvents();
    }
    
    ofAddListener(ofEvents().mouseReleased, this, &ofxOscMapper::mouseReleased);
    ofAddListener(ofEvents().keyPressed, this, &ofxOscMapper::keyPressed);
}


void ofxOscMapper::disableMappingMode(){
    if(!bEnabled || !bMapModeEnabled){
        return;
    }
    
    bMapModeEnabled = false;
    selected = "";
    
    for(int i = 0; i < panels.size(); ++i){
        panels[i]->registerMouseEvents();
    }
    
    ofRemoveListener(ofEvents().mouseReleased, this, &ofxOscMapper::mouseReleased);
    ofRemoveListener(ofEvents().keyPressed, this, &ofxOscMapper::keyPressed);
}


void ofxOscMapper::toggleMappingMode(){
    if(bMapModeEnabled){
        disableMappingMode();
    }else{
        enableMappingMode();
    }
}


void ofxOscMapper::enable(){
    if(!bEnabled){
        ofAddListener(ofEvents().update, this, &ofxOscMapper::update);
        ofAddListener(ofEvents().draw, this, &ofxOscMapper::draw);
        bEnabled = true;
    }
}


void ofxOscMapper::disable(){
    if(bEnabled){
        ofRemoveListener(ofEvents().update, this, &ofxOscMapper::update);
        ofRemoveListener(ofEvents().draw, this, &ofxOscMapper::draw);
        
        disableMappingMode();
        
        bEnabled = false;
    }
}


void ofxOscMapper::loadOscMapping(){
    ofXml XML;
    XML.load("osc-map.xml");
    
    if(XML.exists("link")){
        XML.setTo("link[0]");
        
        do{
            string addressName = XML.getValue<string>("address");
            string controlName = XML.getValue<string>("param");
            
            // check if we've loaded a valid mapping
            // the parameter name loaded from the xml must match
            // the a control names added through addPanel()
            if(controls.find(controlName) == controls.end()) {
                cout << "[ alert ] ofxOscMapper: loaded control [" << controlName << "] hasen't been added" << endl;
            }else{
                addressToName[addressName] = controlName;
            }
        }
        while( XML.setToSibling() ); // go to the next link
    }
}


void ofxOscMapper::saveOscMapping(){
    int num_links = 0;
    
    ofXml XML;
    XML.addChild("map");
    XML.setTo("map");
        
    for(map<string,string>::iterator it=addressToName.begin(); it!=addressToName.end(); ++it){
        string addressName = it->first;
        string controlName = it->second;
        
        XML.setTo("//map");
        XML.addChild("link");
        XML.setTo("link[" + ofToString(num_links) + "]");
        XML.addValue("address", addressName);
        XML.addValue("param", controlName);
        num_links++;
    }
    
    XML.save("osc-map.xml");
}


// are this two objects compatible?
bool ofxOscMapper::matchTypes(string controlName, ofxOscMessage message) {
    ofAbstractParameter& p = controls[controlName]->getParameter();
    
    // vezer can send these types of messages:
    // float, int, bool, color and... string?
    if(message.getNumArgs() == 3){
        if(p.type() == typeid(ofParameter<ofColor>).name()
           && message.getArgType(0) == OFXOSC_TYPE_INT32){
            return true;
        }
        else if(p.type() == typeid(ofParameter<ofFloatColor>).name()
                && message.getArgType(0) == OFXOSC_TYPE_FLOAT){
            return true;
        }
    }
    else if(message.getNumArgs() == 1){
        if(p.type() == typeid(ofParameter<int>).name()
           && message.getArgType(0) == OFXOSC_TYPE_INT32){
            return true;
        }
        else if(p.type() == typeid(ofParameter<float>).name()
                && message.getArgType(0) == OFXOSC_TYPE_FLOAT){
            return true;
        }
        else if(p.type() == typeid(ofParameter<bool>).name()
                && message.getArgType(0) == OFXOSC_TYPE_INT32){
            return true;
        }
        ////////////////////////////////////////////////////////////////
        // testing:
        // floats will be converted to bool if parameter type is bool
        else if(p.type() == typeid(ofParameter<bool>).name()
                && message.getArgType(0) == OFXOSC_TYPE_FLOAT){
            return true;
        }
        // floats will be converted to ints if parameter type is int
        else if(p.type() == typeid(ofParameter<int>).name()
                 && message.getArgType(0) == OFXOSC_TYPE_FLOAT){
            return true;
        }
        ////////////////////////////////////////////////////////////////
    }
    return false;
}


bool ofxOscMapper::isMappeable(ofAbstractParameter & p) {
    // at the moment, there's no way to control multi-value parameters (colors, vectors, etc) with midi or osc
    // a workarround is to expose the color components (r,g,b) as individual parameters
    // eg: color1_r, color1_g, color1_b and color2_r, color2_g, color2_b
    if(p.type() == typeid(ofParameter<float>).name()
       || p.type() == typeid(ofParameter<int>).name()
       || p.type() == typeid(ofParameter<bool>).name()){
        return true;
    }
    return false;
}


bool ofxOscMapper::isGroup(ofAbstractParameter& p){
    if(p.type() == typeid(ofParameterGroup).name()){
        return true;
    }
    return false;
}


void ofxOscMapper::drawOverlay(ofxBaseGui *control, ofColor fillColor, ofColor lineColor){
    ofRectangle r = control->getShape();
    ofFill();
    ofSetColor(fillColor);
    ofRect(r);
    //ofNoFill();
    //ofSetColor(lineColor);
    //ofRect(r);
}


bool ofxOscMapper::isParameterSyncEnabled(){
    return bParamenterSyncEnabled;
}


void ofxOscMapper::toggleParameterSync(){
    if(bParamenterSyncEnabled){
        disableParameterSync();
    }else{
        enableParameterSync();
    }
}


void ofxOscMapper::enableParameterSync(){
    if(bParamenterSyncEnabled){
        return;
    }
    bParamenterSyncEnabled = true;
    
    // we need this because a group can only have one parent
    // because the parent of this parameter might not be
    // the panel group, we need to find the current parent.
    // with multiple parents we could just have added the listener to the panel.
    
    // iterate through the stored controls
    for(map<string,ofxBaseGui*>::iterator it=controls.begin(); it!=controls.end(); ++it){
        string controlName = it->first;
        ofxBaseGui* control = it->second;
        string oscAddress = getAddressByControlName(controlName);
        ofAbstractParameter& parameter = control->getParameter();
        
        // if this control is mapped to an address, create a link between
        // the parameter name and the osc address
        if(oscAddress != ""){
            string paramName = getParameterName(parameter);
            paramNameToAddress[paramName] = oscAddress;
        }
        
        // what is the last parent of this param?
        ofParameterGroup* lastParent = parameter.getParent();
        while(lastParent->getParent()){
            lastParent = lastParent->getParent();
        }
        lastParents[ lastParent->getName() ] = lastParent; // use a map to store the lastParent of the parameter
    }
    
    // add listerner
    for(map<string,ofParameterGroup*>::iterator it=lastParents.begin(); it!=lastParents.end(); ++it){
        ofParameterGroup* lastParent = it->second;
        cout << lastParent->getName() << endl;
        ofAddListener(lastParent->parameterChangedE, this, &ofxOscMapper::parameterChanged);
    }
}


void ofxOscMapper::disableParameterSync(){
    if(!bParamenterSyncEnabled){
        return;
    }
    bParamenterSyncEnabled = false;
    
    for(map<string,ofParameterGroup*>::iterator it=lastParents.begin(); it!=lastParents.end(); ++it){
        ofParameterGroup* lastParent = it->second;
        ofRemoveListener(lastParent->parameterChangedE, this, &ofxOscMapper::parameterChanged);
    }
    lastParents.clear();
}


void ofxOscMapper::parameterChanged(ofAbstractParameter & parameter){
    string paramName = getParameterName(parameter);
    string address = getAddressByParamName(paramName);
    
    if(address == ""){
        return;
    }
    
    ofxOscMessage msg;
    msg.setAddress(address);
    
    // NOTE: all parameters are sent as float args
    if(parameter.type()==typeid(ofParameter<int>).name()){
        msg.addFloatArg(parameter.cast<int>());
    }else if(parameter.type()==typeid(ofParameter<float>).name()){
        msg.addFloatArg(parameter.cast<float>());
    }else if(parameter.type()==typeid(ofParameter<bool>).name()){
        msg.addFloatArg(parameter.cast<bool>());
    }
    
    // send message
//    for (int i = 0; i < oscOut.size(); i++) {
//        oscOut[i]->sendMessage(msg);
//    }
    
//    senderTouchOSC.sendMessage(msg);
}


string ofxOscMapper::getParameterName(ofAbstractParameter &parameter){
    string paramName = "";
    const vector<string> hierarchy = parameter.getGroupHierarchyNames();
    for(int i=0; i<(int)hierarchy.size()-1; i++){
        paramName += hierarchy[i] + ".";
    }
    paramName += parameter.getEscapedName();

    return paramName;
}


string ofxOscMapper::getAddressByControlName(string controlName){
    string address = "";
    for(map<string,string>::iterator it=addressToName.begin(); it!=addressToName.end(); ++it){
        if(it->second == controlName){
            address = it->first;
            break;
        }
    }
    return address;
}


string ofxOscMapper::getAddressByParamName(string paramName){
    string address = "";
    if(paramNameToAddress.find(paramName) != paramNameToAddress.end()){
        address = paramNameToAddress[paramName];
    }
    return address;
}
