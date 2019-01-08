//
//  ColorScheme.h
//
//  Created by Brian on 9/13/18.
//
//

#pragma once

#include "ofMain.h"

class ColorScheme {
public:
    
    ColorScheme() {}
    ~ColorScheme(){}
    
    void setBgColor(ofColor bg) {
        bg_color = bg;
    }
    
	void setColors(vector<ofColor> scheme) {
		for (const auto& color : scheme)
		{
			colors.push_back(color);
		}
		numColors = int(colors.size());
	}

	void addColor(ofColor color) {
		colors.push_back(color);
		numColors = int(colors.size());
	}
    
    ofColor bg_color;
	vector<ofColor> colors;
    
	int numColors;
};
