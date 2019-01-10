#pragma once

#include "ofMain.h"

//	Application
#include "ColorScheme.h"
#include "NP_Particles.h"
#include "SP_Particles.h"

//	Addons
#include "ofxGLWarper.h"
#include "ofxKinectForWindows2.h"
#include "ofxOsc.h"

#define KD_WIDTH 512.0
#define KD_HEIGHT 424.0

#define HOST "192.168.0.99"
#define PORT0 10000
#define PORT1 10001
#define PORT2 10002

class ofApp : public ofBaseApp{

public:
	void setup();
	void loadAppSettings();
	void update();
	void draw();

	//	Behavoir
	void resetParticles();
	void resetColors();

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
	//	Interaction Data
	//*********************************
	float xOrigin, yOrigin, xMin, yMin, xMax, yMax;

	//*********************************
	//	Kinect
	//*********************************
	ofxKFW2::Device kinect;
	ofPoint *lHand, *rHand;
	float k_xscale, k_yscale;
	float k_xoffset, k_yoffset;

	//*********************************
	//	Noise particles
	//*********************************
	vector<NP_Particles> particleSystem;
	int numSystems;

	//*********************************
	//	Space particles
	//*********************************
	SP_Particles sp_particles;
	const int sp_x_dim = 750;
	const int sp_y_dim = 750;

	//*********************************
	//	Colors
	//*********************************
	vector<ColorScheme> colorSchemes;
	int activeScheme;
	int numSchemes;

	//*********************************
	//	Osc
	//*********************************
	ofxOscSender oscSend0;
	ofxOscSender oscSend1;
	ofxOscSender oscSend2;
	float leftx, lefty, rightx, righty;

	//*********************************
	//	Scene
	//*********************************
	float resetTime, colorTime, sceneTime;
	float resetPeriod, colorPeriod, scenePeriod;

	typedef enum
	{
		NOISE_PARTICLES = 0,
		SLOW_PARTICLES = 1,
		NUM_SCENES
	} Scene;

	Scene scene;
};
