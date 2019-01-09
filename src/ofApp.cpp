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
	for (auto& particles : particleSystem)
	{
		int colorIndex = i % colorSchemes[activeScheme].numColors;
		particles.bgColor = colorSchemes[activeScheme].bg_color;
		particles.fgColor = colorSchemes[activeScheme].colors.at(colorIndex);
		particles.initColorTexture();
		++i;
	}

	//*********************************
	//	FBO Allocation
	//*********************************
	fboOut.allocate(w, h, GL_RGBA32F_ARB);
	fboOut.begin();
	ofClear(255);
	fboOut.end();

	//*********************************
	//	Kinect
	//*********************************
	kinect.open();
	kinect.initBodySource();
	k_xscale = w / KD_WIDTH;
	k_yscale = h / KD_HEIGHT;
	k_xoffset = 0.0f;
	k_yoffset = -0.3 * h;

	//*********************************
	//	Osc
	//*********************************
	oscSend0.setup(HOST, PORT0);
	oscSend1.setup(HOST, PORT1);
	oscSend2.setup(HOST, PORT2);

	//*********************************
	//	Scene
	//********************************
	resetPeriod = 20.0f;
	colorPeriod = 90.0f;

	resetTime = ofGetElapsedTimef() + resetPeriod;
	colorTime = ofGetElapsedTimef() + colorPeriod;
}

//--------------------------------------------------------------
void ofApp::update(){

	//*********************************
	//  Data
	//*********************************
	t = ofGetElapsedTimef();
	f = ofGetFrameNum();

	if (t >= resetTime)
	{
		resetTime = t + (resetPeriod * ofRandom(0.9, 1.1));
		resetParticles();
	}
	if (t >= colorTime)
	{
		colorTime = t + (colorPeriod * ofRandom(0.9, 1.1));
		resetColors();
	}

	//*********************************
	//	Interaction Data
	//*********************************
	float xOrigin, yOrigin, xMin, yMin, xMax, yMax;
	xOrigin = 0.5 * w;
	yOrigin = 0.5 * h;
	xMin = 0;
	yMin = 0;
	xMax = 0.5 * w;
	yMax = 0.5 * h;

	//*********************************
	//	Kinect
	//*********************************
	kinect.update();
	auto bodies = kinect.getBodySource()->getBodies();
	
	lHand = NULL;
	rHand = NULL;

	for(auto body : bodies)
	{
		if (body.joints.size() < 20)
		{
			continue;
		}

		auto leftHand = body.joints.at(JointType_HandLeft);
		auto rightHand = body.joints.at(JointType_HandRight);

		lHand = &ofPoint(leftHand.getPositionInDepthMap().x * k_xscale + k_xoffset, leftHand.getPositionInDepthMap().y * k_yscale + k_yoffset, 0);
		rHand = &ofPoint(rightHand.getPositionInDepthMap().x * k_xscale + k_xoffset, rightHand.getPositionInDepthMap().y * k_yscale + k_yoffset, 0);

		break;
	}

	//*********************************
	//	Osc
	//*********************************
	if (lHand != NULL)
	{
		leftx = abs(lHand->x - xOrigin);
		lefty = abs(lHand->y - yOrigin);
	}
	if (rHand != NULL)
	{
		rightx = abs(rHand->x - xOrigin);
		righty = abs(rHand->y - yOrigin);
	}

	float dramaL = 0.5f;
	float colorL = 0.5f;
	float pmbL = 0.5f;

	float cutoffR = 0.3;

	if (lHand != NULL)
	{
		dramaL = ofMap(leftx, xMin, xMax, 0.0, 1.0, true);
		colorL = ofMap(leftx, xMin, xMax, 0.5, 1.0, true);
		pmbL = ofMap(leftx, xMin, xMax, 0.5, 0.65, true);
	}
	if (rHand != NULL)
	{
		cutoffR = ofMap(rightx, xMin, xMax, 0.2, 0.6, true);
	}
	else
	{
		
	}

	//***	Left hand
	ofxOscMessage m;
	m.setAddress("/dramaL");
	m.addFloatArg(dramaL);
	oscSend0.sendMessage(m);

	m.clear();
	m.setAddress("/colorL");
	m.addFloatArg(colorL);

	oscSend0.sendMessage(m);

	m.clear();
	m.setAddress("/pmbL");
	m.addFloatArg(pmbL);

	oscSend0.sendMessage(m);

	//***	Right hand
	m.clear();
	m.setAddress("/cutoffR");
	m.addFloatArg(cutoffR);

	oscSend1.sendMessage(m);
	
	//*********************************
	//	Particles
	//*********************************
	for (auto& particles : particleSystem)
	{
		particles.leftPos = lHand;
		particles.rightPos = rHand;
		particles.update();
	}

	//*********************************
	//	Window
	//*********************************
	ofSetWindowTitle(ofToString(ofGetFrameRate()));
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

	//*********************************
	//	Kinect
	//*********************************
	kinect.getBodySource()->drawProjected(0, 0, w, h);
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

		auto bgcolor = cScheme.getChild("bgcolor");
		bgc.r = bgcolor.getChild("r").getFloatValue();
		bgc.g = bgcolor.getChild("g").getFloatValue();
		bgc.b = bgcolor.getChild("b").getFloatValue();
		bgc.a = bgcolor.getChild("a").getFloatValue();
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
void ofApp::resetParticles() {
	for (auto& particles : particleSystem)
	{
		particles.reset();
	}
}

//--------------------------------------------------------------
void ofApp::resetColors() {
	activeScheme = ++activeScheme % int(colorSchemes.size());

	int i = 0;
	for (auto& particles : particleSystem)
	{
		int colorIndex = i % colorSchemes[activeScheme].numColors;
		particles.bgColor = colorSchemes[activeScheme].bg_color;
		particles.fgColor = colorSchemes[activeScheme].colors.at(colorIndex);
		particles.initColorTexture();
		++i;
	}
}

//--------------------------------------------------------------
void ofApp::reloadShaders() {
	for (auto& particles : particleSystem)
	{
		particles.reloadShaders();
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	switch (key)
	{
	case 'c':
		resetColors();
		break;
	case 'r':
		resetParticles();
		break;
	case 'R':
		reloadShaders();
		break;
	default:
		break;
	}
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

	//*********************************
	//  Data
	//*********************************
	this->w = w;
	this->h = h;

	//*********************************
	//	Particles
	//*********************************
	for (auto& particles : particleSystem)
	{
		particles.resize();
	}

	//*********************************
	//	FBO Allocation
	//*********************************
	fboOut.allocate(w, h, GL_RGBA32F_ARB);
	fboOut.begin();
	ofClear(255);
	fboOut.end();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
