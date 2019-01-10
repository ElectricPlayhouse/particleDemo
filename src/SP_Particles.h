//
//  NP_Particles.h
//  gpuParticleSystem
//
//  Created by Brian on 8/4/18.
//
//

#pragma once

#include "ofMain.h"

class SP_Particles
{
public:

	SP_Particles();
	~SP_Particles();

	//  Texture enum
	enum TextureIndex
	{
		POSITION,
		VELOCITY,
		COLOR,
		SPEED
	};

	//  Constants
	const string UPDATE_SHADER_NAME = "sp_update";
	const string DRAW_SHADER_NAME = "sp_draw";

	//  Core methods
	void setup(unsigned int x_count, unsigned int y_count, ofPrimitiveMode primitive_mode = OF_PRIMITIVE_POINTS, unsigned int num_textures = 4);
	void update();
	void draw();
	void resize();
	void reset();
	void reloadShaders();

	//  Initialization
	void loadTexture(unsigned int index, float* texture);
	void initPositionTexture();
	void initVelocityTexture();
	void initColorTexture();
	void initColorTexture(ofColor color);
	void initSpeedTexture();
	void clearTexture(unsigned int index);

	float * initialPositionData;
	float * initialVelocityData;

	//  Set functions
	void setAlphaFactor(float factor);

	//	Public Data
	float r;
	ofColor bgColor;
	ofColor fgColor1;
	ofColor fgColor2;

	ofPoint *leftPos, *rightPos;

private:

	//	Data
	float w, h, t;
	float f;

	//  Fbos
	ofFbo fbos[2];

	//  Meshes
	ofVboMesh particle_mesh, bg_mesh;

	//Shaders
	ofShader update_shader, draw_shader;

	//  Texture
	unsigned int current_fbo_index;
	unsigned int x_dim, y_dim;

	//  Noise
	float noise_scale, noise_frequency;

	//  Interaction
	ofPoint targetPos;
	ofPoint currentPos;
	float scaleX, scaleY;
	float centroidX, centroidY;
	bool bCentroid;

	//  Color
	float alpha_factor;
};
