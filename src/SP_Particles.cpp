//
//  SP_Particles.cpp
//  gpuParticleSystem
//
//  Created by Brian on 8/4/18.
//
//

#include "SP_Particles.h"

//--------------------------------------------------------------
SP_Particles::SP_Particles() :
	current_fbo_index(0),
	alpha_factor(1.0)
{

}

//--------------------------------------------------------------
SP_Particles::~SP_Particles()
{

}

//--------------------------------------------------------------
void SP_Particles::setup(unsigned int x_count, unsigned int y_count, ofPrimitiveMode primitive_mode, unsigned int num_textures)
{
	//	Data
	w = ofGetWidth();
	h = ofGetHeight();

	x_dim = x_count;
	y_dim = y_count;

	//  Fbo setup
	ofFbo::Settings s;
	s.internalformat = GL_RGBA32F_ARB;
	s.textureTarget = GL_TEXTURE_RECTANGLE_ARB;
	s.minFilter = GL_NEAREST;
	s.maxFilter = GL_NEAREST;
	s.wrapModeHorizontal = GL_CLAMP;
	s.wrapModeVertical = GL_CLAMP;
	s.width = x_dim;
	s.height = y_dim;
	s.numColorbuffers = num_textures;

	for (unsigned i = 0; i < 2; ++i)
	{
		fbos[i].allocate(s);
	}

	//  Particle mesh setup
	particle_mesh.clear();

	for (int y = 0; y < y_dim; ++y)
	{
		for (int x = 0; x < x_dim; ++x)
		{
			particle_mesh.addVertex(ofVec3f(200.f * x / (float)x_dim - 100.f, 200.f * y / (float)y_dim - 100.f, -500.f));
			particle_mesh.addTexCoord(ofVec2f(x, y));
		}
	}
	particle_mesh.setMode(primitive_mode);

	//  BG mesh setup
	bg_mesh.clear();
	bg_mesh.addVertex(ofVec3f(-1.f, -1.f, 0.f));
	bg_mesh.addVertex(ofVec3f(1.f, -1.f, 0.f));
	bg_mesh.addVertex(ofVec3f(1.f, 1.f, 0.f));
	bg_mesh.addVertex(ofVec3f(-1.f, 1.f, 0.f));

	bg_mesh.addTexCoord(ofVec2f(0.f, 0.f));
	bg_mesh.addTexCoord(ofVec2f(x_dim, 0.f));
	bg_mesh.addTexCoord(ofVec2f(x_dim, y_dim));
	bg_mesh.addTexCoord(ofVec2f(0.f, y_dim));

	bg_mesh.addIndex(0);
	bg_mesh.addIndex(1);
	bg_mesh.addIndex(2);
	bg_mesh.addIndex(0);
	bg_mesh.addIndex(2);
	bg_mesh.addIndex(3);

	bg_mesh.setMode(OF_PRIMITIVE_TRIANGLES);

	//  Shaders
	update_shader.load("shaders/" + UPDATE_SHADER_NAME);
	draw_shader.load("shaders/" + DRAW_SHADER_NAME);

	//  Texture setup
	initPositionTexture();
	initVelocityTexture();
	initColorTexture();
	initSpeedTexture();

	//  Interaction
	scaleX = float(ofGetWidth()) / 512.0f;
	scaleY = float(ofGetHeight()) / 424.0f;

	targetPos = currentPos = ofPoint(0.5 * w, 0.5 * h, 0);
}

//--------------------------------------------------------------
void SP_Particles::update()
{
	//	Data
	t = ofGetElapsedTimef();
	f = ofGetFrameNum();

	//  Interaction
	if (leftPos != NULL && rightPos != NULL) {
		targetPos.x = 0.5 * (leftPos->x + rightPos->x);
		targetPos.y = 0.5 * (leftPos->y + rightPos->y);
	}
	else {
		targetPos.x = w * ofNoise(0.01 * t);
		targetPos.y = h * ofNoise(0.012 * t);
	}

	currentPos += 0.002 * (targetPos - currentPos);

	noise_scale = ofMap(currentPos.x, 0, w, 200, 800, true);
	noise_frequency = ofMap(currentPos.y, 0, h, 1.0, 32.0, true);

	//	Particles update
	fbos[1 - current_fbo_index].begin(false);

	glPushAttrib(GL_ENABLE_BIT);
	glViewport(0, 0, x_dim, y_dim);
	glDisable(GL_BLEND);
	ofSetColor(255, 255, 255);
	fbos[1 - current_fbo_index].activateAllDrawBuffers();

	update_shader.begin();

	update_shader.setUniform1f("elapsed", ofGetLastFrameTime());
	update_shader.setUniform1f("radiusSquared", 250000.0f);
	update_shader.setUniform2f("resolution", ofVec2f(ofGetWidth(), ofGetHeight()));

	if (leftPos != NULL)
		update_shader.setUniform3f("leftPos", *leftPos);
	else
		update_shader.setUniform3f("leftPos", 99999999, 99999999, 99999999);
	if (rightPos != NULL)
		update_shader.setUniform3f("rightPos", *rightPos);
	else
		update_shader.setUniform3f("rightPos", 99999999, 99999999, 99999999);

	for (unsigned i = 0; i < fbos[current_fbo_index].getNumTextures(); ++i)
	{
		ostringstream stream;
		stream << "particles" << ofToString(i);
		update_shader.setUniformTexture(stream.str().c_str(), fbos[current_fbo_index].getTexture(i), i);
	}

	bg_mesh.draw();
	update_shader.end();
	glPopAttrib();

	fbos[1 - current_fbo_index].end();

	current_fbo_index = 1 - current_fbo_index;
}

//--------------------------------------------------------------
void SP_Particles::draw()
{
	glPointSize(2.0);
	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_BLEND);

	draw_shader.begin();

	draw_shader.setUniform1f("radius", r);

	for (unsigned i = 0; i < fbos[current_fbo_index].getNumTextures(); ++i)
	{
		ostringstream stream;
		stream << "particles" << ofToString(i);
		draw_shader.setUniformTexture(stream.str().c_str(), fbos[current_fbo_index].getTexture(i), i);
	}

	particle_mesh.draw();
	draw_shader.end();
}

//--------------------------------------------------------------
void SP_Particles::loadTexture(unsigned int index, float* texture)
{
	if (index < fbos[current_fbo_index].getNumTextures())
	{
		fbos[current_fbo_index].getTexture(index).bind();
		glTexSubImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, 0, 0, x_dim, y_dim, GL_RGBA, GL_FLOAT, texture);
		fbos[current_fbo_index].getTexture(index).unbind();
	}
	else
	{
		ofLogError() << "Trying to load texture into non-existent buffer.";
	}
}

//--------------------------------------------------------------
void SP_Particles::initPositionTexture()
{
	initialPositionData = new float[x_dim * y_dim * 4]; //4 for RGBA

	for (unsigned int y = 0; y < y_dim; ++y)
	{
		for (unsigned int x = 0; x < x_dim; ++x)
		{
			unsigned int index = y * x_dim + x;
			initialPositionData[index * 4 + 0] = w * x / (float)x_dim;
			initialPositionData[index * 4 + 1] = h * y / (float)y_dim;
			initialPositionData[index * 4 + 2] = 0.0f;
			initialPositionData[index * 4 + 3] = 0.0f;
		}
	}

	loadTexture(POSITION, initialPositionData);

	return;
}

//--------------------------------------------------------------
void SP_Particles::initVelocityTexture()
{
	clearTexture(VELOCITY);
	initialVelocityData = new float[x_dim * y_dim * 4];
	memset(initialVelocityData, 0, sizeof(float) * x_dim * y_dim * 4);
}

//--------------------------------------------------------------
void SP_Particles::initColorTexture()
{
	float* col_tex = new float[x_dim * y_dim * 4]; //4 for RGBA

	for (unsigned y = 0; y < y_dim; ++y)
	{
		for (unsigned x = 0; x < x_dim; ++x)
		{
			unsigned index = y * x_dim + x;

			float alpha = ofMap(index, 0.0, x_dim * y_dim, 0.0, 0.5);
			float pctX = ofMap(x, 0, x_dim, 0, 1);
			float pctY = ofMap(y, 0, y_dim, 0, 1);
			float pctZ = ofMap(index, 0, x_dim * y_dim, 0, 1);

			col_tex[index * 4 + 0] = ofLerp(fgColor1.r, fgColor2.r, pctX) / 255.0;
			col_tex[index * 4 + 1] = ofLerp(fgColor1.g, fgColor2.g, pctY) / 255.0;
			col_tex[index * 4 + 2] = ofLerp(fgColor1.b, fgColor2.b, pctZ) / 255.0;
			col_tex[index * 4 + 3] = alpha * alpha_factor;
		}
	}

	loadTexture(COLOR, col_tex);
	delete[] col_tex;

	return;
}

//--------------------------------------------------------------
void SP_Particles::initColorTexture(ofColor color)
{
	float* col_tex = new float[x_dim * y_dim * 4]; //4 for RGBA

	for (unsigned y = 0; y < y_dim; ++y)
	{
		for (unsigned x = 0; x < x_dim; ++x)
		{
			unsigned index = y * x_dim + x;

			float alpha = ofMap(index, 0.0, x_dim * y_dim, 0.0, 1.0);

			col_tex[index * 4 + 0] = color.r / 255.0;
			col_tex[index * 4 + 1] = color.g / 255.0;
			col_tex[index * 4 + 2] = color.b / 255.0;
			col_tex[index * 4 + 3] = alpha * alpha_factor;
		}
	}

	loadTexture(COLOR, col_tex);
	delete[] col_tex;

	return;
}

//--------------------------------------------------------------
void SP_Particles::initSpeedTexture()
{
	float* spd_tex = new float[x_dim * y_dim * 4]; //4 for RGBA

	for (unsigned y = 0; y < y_dim; ++y)
	{
		for (unsigned x = 0; x < x_dim; ++x)
		{
			unsigned index = y * x_dim + x;
			spd_tex[index * 4 + 0] = ofRandom(0.1, 2.0);
			spd_tex[index * 4 + 1] = 0.0f;
			spd_tex[index * 4 + 2] = 0.0f;
			spd_tex[index * 4 + 3] = 0.0f;
		}
	}

	loadTexture(SPEED, spd_tex);
	delete[] spd_tex;

	return;
}

//--------------------------------------------------------------
void SP_Particles::clearTexture(unsigned int index)
{
	float* tex = new float[x_dim * y_dim * 4];
	memset(tex, 0, sizeof(float) * x_dim * y_dim * 4);

	loadTexture(index, tex);

	delete[] tex;

	return;
}

//--------------------------------------------------------------
void SP_Particles::setAlphaFactor(float factor)
{
	alpha_factor = factor;
}

//--------------------------------------------------------------
//void SP_Particles::setInteractionPoints(ofPoint* leftPos, ofPoint* rightPos)
//{
//	this->leftPos = leftPos;
//	this->rightPos = rightPos;
//}

//--------------------------------------------------------------
void SP_Particles::resize()
{
	//	Data
	w = ofGetWidth();
	h = ofGetHeight();
}

//--------------------------------------------------------------
void SP_Particles::reset()
{
	targetPos = currentPos = ofPoint(ofRandom(w), ofRandom(h), 0);

	loadTexture(POSITION, initialPositionData);
	loadTexture(VELOCITY, initialVelocityData);
	initColorTexture();
}

//--------------------------------------------------------------
void SP_Particles::reloadShaders()
{
	update_shader.load("shaders/" + UPDATE_SHADER_NAME);
	draw_shader.load("shaders/" + DRAW_SHADER_NAME);
}