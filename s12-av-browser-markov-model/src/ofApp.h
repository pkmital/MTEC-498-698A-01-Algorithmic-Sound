#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxMaxim.h"
#include "ofxTSNE.h"
#include "CircularBuffer.h"
#include "OnsetDetector.h"
#include "Recording.h"
#include "AudioVisualCorpus.h"


class ofApp : public ofBaseApp{

	public:
		void setup();
		void draw();
		void mouseMoved(int x, int y );
        void audioOut(float *buf, int, int);
        void keyPressed(int k);
    
    private:
        int width, height;
        ofEasyCam cam;
        AudioVisualCorpus corpus;
        maxiSample sample;
        OnsetDetector detector;
        AudioVisualCorpus::SYNTH_MODE mode;
};
