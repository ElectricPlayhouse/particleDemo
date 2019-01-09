#pragma once

#include "ofMain.h"

//	Application
#include "ColorScheme.h"
#include "NP_Particles.h"

//	Addons
#include "ofxGLWarper.h"
#include "ofxKinectForWindows2.h"
#include "ofxOsc.h"

#define KD_WIDTH 512.0
#define KD_HEIGHT 424.0

class ofApp : public ofBaseApp{

public:
	void setup();
	void loadAppSettings();
	void update();
	void draw();

	//	Behavoir
	void resetParticles();

	//	UI
	void reloadShaders();

	void keyPressed(int key);
	void keyReleased(int key);
	void mouseMoved(int x, int y );
	void mouseDragged(int x, int y, int button);
	void mousePressed(int x, int y, int button);
	void mouseReleased(int x, int y, int button);
	void mouseEntered(int x, int y);
	void mouseExited(int x, int y);
	void windowResized(int w, int h);
	void dragEvent(ofDragInfo dragInfo);
	void gotMessage(ofMessage msg);
	
	//*********************************
	//	Data
	//*********************************
	float w, h, t;
	int f;

	//*********************************
	//	Display
	//*********************************
	ofFbo fboOut;

	//*********************************
	//	Kinect
	//*********************************
	ofxKFW2::Device kinect;
	ofPoint *lHand, *rHand;
	float k_xscale, k_yscale;

	//*********************************
	//	Noise particles
	//*********************************
	vector<NP_Particles> particleSystem;
	vector<ColorScheme> colorSchemes;
	int activeScheme;
	int numSystems, numSchemes;

	//*********************************
	//	Scene
	//*********************************
	float resetTime, colorTime;
	float resetPeriod, colorPeriod;
};
