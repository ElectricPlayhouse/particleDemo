#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	//*********************************
	//  OF Settings
	//*********************************
	ofSetFrameRate(60);
	//ofSetFullscreen(true);

	//*********************************
	//  Data
	//*********************************
	w = ofGetWidth();
	h = ofGetHeight();

	//*********************************
	//	App Settings
	//*********************************
	loadAppSettings();

	//*********************************
	//	Particles
	//*********************************
	activeScheme = 0;
	int i = 0;
	for (auto particles : particleSystem)
	{
		int colorIndex = i % colorSchemes[activeScheme].numColors;
		particles.bgColor = colorSchemes[activeScheme].bg_color;
		particles.initColorTexture(colorSchemes[activeScheme].colors.at(colorIndex));
		++i;
	}

	//*********************************
	//	FBO Allocation
	//*********************************
	fboOut.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F_ARB);
	fboOut.begin();
	ofClear(255);
	fboOut.end();
}

//--------------------------------------------------------------
void ofApp::update(){

	//*********************************
	//  Data
	//*********************************
	t = ofGetElapsedTimef();
	f = ofGetFrameNum();

	//*********************************
	//	Particles
	//*********************************
	for (auto& particles : particleSystem)
	{
		particles.update();
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	//*********************************
	//	Background
	//*********************************
	ofBackground(colorSchemes[activeScheme].bg_color);

	//*********************************
	//	Draw to Fbo Out
	//*********************************
	fboOut.begin();

	ofEnableBlendMode(OF_BLENDMODE_ALPHA);

	ofSetColor(colorSchemes[activeScheme].bg_color, 8);
	ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

	for (auto& particles : particleSystem)
	{
		particles.draw();
	}

	ofDisableBlendMode();

	fboOut.end();

	fboOut.draw(0, 0);
}

//--------------------------------------------------------------
void ofApp::loadAppSettings() {

	//*********************************
	//	Particle Settings
	//*********************************
	ofXml XML;
	if (!XML.load("particleSettings.xml")) {
		ofLog(OF_LOG_ERROR, "loadAppSettings : particle xml file not loaded properly. Check file path.");
		return;
	}

	auto pSystems = XML.getChildren("psystem");
	for (auto& pSystem : pSystems)
	{
		NP_Particles np;

		float xdim = pSystem.getChild("xdim").getFloatValue();
		float ydim = pSystem.getChild("ydim").getFloatValue();
		float radius = pSystem.getChild("radius").getFloatValue();

		np.setup(xdim, ydim);
		np.r = radius;

		particleSystem.push_back(np);
	}

	numSystems = int(particleSystem.size());

	//*********************************
	//	Color Settings
	//*********************************
	if (!XML.load("colorSettings.xml")) {
		ofLog(OF_LOG_ERROR, "loadAppSettings : color xml file not loaded properly. Check file path.");
		return;
	}

	int numSchemes = 0;
	auto cSchemes = XML.getChildren("scheme");
	for (auto& cScheme : cSchemes)
	{
		ColorScheme scheme;

		ofColor bgc;
		bgc.r = cScheme.getChild("r").getFloatValue();
		bgc.g = cScheme.getChild("g").getFloatValue();
		bgc.b = cScheme.getChild("b").getFloatValue();
		bgc.a = cScheme.getChild("a").getFloatValue();
		scheme.setBgColor(bgc);

		auto colors = cScheme.getChildren("color");
		for (auto& color : colors)
		{
			ofColor c;
			c.r = color.getChild("r").getFloatValue();
			c.g = color.getChild("g").getFloatValue();
			c.b = color.getChild("b").getFloatValue();
			c.a = color.getChild("a").getFloatValue();
			scheme.addColor(c);
		}

		colorSchemes.push_back(scheme);
	}

	numSchemes = int(colorSchemes.size());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
