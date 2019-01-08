//
//  NP_Particles.cpp
//  gpuParticleSystem
//
//  Created by Brian on 8/4/18.
//
//

#include "NP_Particles.h"

//--------------------------------------------------------------
NP_Particles::NP_Particles() :
current_fbo_index(0),
alpha_factor(1.0)
{
    
}

//--------------------------------------------------------------
NP_Particles::~NP_Particles()
{
    
}

//--------------------------------------------------------------
void NP_Particles::setup(unsigned int x_count, unsigned int y_count, ofPrimitiveMode primitive_mode, unsigned int num_textures)
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
    
    for(unsigned i = 0; i < 2; ++i)
    {
        fbos[i].allocate(s);
    }
    
    //draw_fbo.allocate(ofGetWidth(), ofGetHeight(), GL_RGBA32F_ARB);
    //draw_fbo.begin();
    //ofClear(255);
    //draw_fbo.end();
    
    //  Particle mesh setup
    particle_mesh.clear();
    
    for(int y = 0; y < y_dim; ++y)
    {
        for(int x = 0; x < x_dim; ++x)
        {
            particle_mesh.addVertex(ofVec3f(200.f * x / (float)x_dim - 100.f, 200.f * y / (float)y_dim - 100.f, -500.f));
            particle_mesh.addTexCoord(ofVec2f(x, y));
        }
    }
    particle_mesh.setMode(primitive_mode);
    
    //  BG mesh setup
    bg_mesh.clear();
    bg_mesh.addVertex(ofVec3f(-1.f, -1.f, 0.f));
    bg_mesh.addVertex(ofVec3f( 1.f, -1.f, 0.f));
    bg_mesh.addVertex(ofVec3f( 1.f,  1.f, 0.f));
    bg_mesh.addVertex(ofVec3f(-1.f,  1.f, 0.f));
    
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
void NP_Particles::update()
{
    //	Data
	t = ofGetElapsedTimef();
	f = ofGetFrameNum();

    //  Interaction
	targetPos.x = ofGetMouseX();
	targetPos.y = ofGetMouseY();
    
    currentPos += 0.002 * (targetPos - currentPos);
    
    noise_scale = ofMap(currentPos.x, 0, ofGetWidth(), 200, 800, true);
    noise_frequency = ofMap(currentPos.y, 0, ofGetHeight(), 1.0, 32.0, true);
    
	//	Particles update
    fbos[1 - current_fbo_index].begin(false);
    
    glPushAttrib(GL_ENABLE_BIT);
    glViewport(0, 0, x_dim, y_dim);
    glDisable(GL_BLEND);
    ofSetColor(255, 255, 255);
    fbos[1 - current_fbo_index].activateAllDrawBuffers();
    
    update_shader.begin();

    update_shader.setUniform1f("elapsed", ofGetLastFrameTime());
    update_shader.setUniform1f("radiusSquared", 160000.0f);
    update_shader.setUniform2f("resolution", ofVec2f(ofGetWidth(), ofGetHeight()));
    
    update_shader.setUniform1f("noise_scale", noise_scale);
    update_shader.setUniform1f("noise_frequency", noise_frequency);
    
    for(unsigned i = 0; i < fbos[current_fbo_index].getNumTextures(); ++i)
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
void NP_Particles::draw()
{
    glPointSize(2.0);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_BLEND);
    
    draw_shader.begin();
    
    draw_shader.setUniform1f("radius", r);
    
    for(unsigned i = 0; i < fbos[current_fbo_index].getNumTextures(); ++i)
    {
        ostringstream stream;
        stream << "particles" << ofToString(i);
        draw_shader.setUniformTexture(stream.str().c_str(), fbos[current_fbo_index].getTexture(i), i);
    }
    
    particle_mesh.draw();
    draw_shader.end();
}

//--------------------------------------------------------------
void NP_Particles::loadTexture(unsigned int index, float* texture)
{
    if(index < fbos[current_fbo_index].getNumTextures())
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
void NP_Particles::initPositionTexture()
{
    float* pos_tex = new float[x_dim * y_dim * 4]; //4 for RGBA
    
    float screen_w = ofGetWidth();
    float screen_h = ofGetHeight();
    
    for(unsigned int y = 0; y < y_dim; ++y)
    {
        for(unsigned int x = 0; x < x_dim; ++x)
        {
            unsigned int index = y * x_dim + x;
            pos_tex[index * 4 + 0] = ofRandom(screen_w);
            pos_tex[index * 4 + 1] = ofRandom(screen_h);
            pos_tex[index * 4 + 2] = 0.0f;
            pos_tex[index * 4 + 3] = 0.0f;
        }
    }
    
    loadTexture(POSITION, pos_tex);
    delete[] pos_tex;
    
    return;
}

//--------------------------------------------------------------
void NP_Particles::initVelocityTexture()
{
    clearTexture(VELOCITY);
}

//--------------------------------------------------------------
void NP_Particles::initColorTexture()
{
    float* col_tex = new float[x_dim * y_dim * 4]; //4 for RGBA
    
    float screen_w = ofGetWidth();
    float screen_h = ofGetHeight();
    
    for(unsigned y = 0; y < y_dim; ++y)
    {
        for(unsigned x = 0; x < x_dim; ++x)
        {
            unsigned index = y * x_dim + x;
            
            float alpha  = ofMap(index, 0.0, x_dim * y_dim, 0.01, 0.5);
            
            col_tex[index * 4 + 0] = 0.33 + 0.67 * float(y) / float(y_dim);
            col_tex[index * 4 + 1] = 0.33 * float(y) / float(y_dim);
            col_tex[index * 4 + 2] = 1.0f - 0.67 * float(x) / float(x_dim);
            col_tex[index * 4 + 3] = alpha * alpha_factor;
        }
    }
    
    loadTexture(COLOR, col_tex);
    delete[] col_tex;
    
    return;
}

//--------------------------------------------------------------
void NP_Particles::initColorTexture(ofColor color)
{
    float* col_tex = new float[x_dim * y_dim * 4]; //4 for RGBA
    
    float screen_w = ofGetWidth();
    float screen_h = ofGetHeight();
    
    for(unsigned y = 0; y < y_dim; ++y)
    {
        for(unsigned x = 0; x < x_dim; ++x)
        {
            unsigned index = y * x_dim + x;
            
            float alpha  = ofMap(index, 0.0, x_dim * y_dim, 0.0, 1.0);
            
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
void NP_Particles::initSpeedTexture()
{
    float* spd_tex = new float[x_dim * y_dim * 4]; //4 for RGBA
    
    for(unsigned y = 0; y < y_dim; ++y)
    {
        for(unsigned x = 0; x < x_dim; ++x)
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
void NP_Particles::clearTexture(unsigned int index)
{
    float* tex = new float[x_dim * y_dim * 4];
    memset(tex, 0, sizeof(float) * x_dim * y_dim * 4);
    
    loadTexture(index, tex);
    delete[] tex;
    
    return;
}

//--------------------------------------------------------------
void NP_Particles::setAlphaFactor(float factor)
{
    alpha_factor = factor;
}