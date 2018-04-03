#include "ofMain.h"
#include "ofxMaxim.h"
#include "maxiGrains.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

    public:
        void setup(){
            speed = rate = 1.0;
            sample1.load(ofToDataPath("amen.wav"));
            
            setupGUI();
            ts = new maxiTimePitchStretch<hannWinFunctor, maxiSample>(&sample1);
            ofSoundStreamSetup(1, 0, 44100, 512, 3);
            maxiSettings::setup(44100, 1, 3);
        }
    
        void setupGUI(){
            gui.setup("Parameters");
            ofParameterGroup g;
            g.setName("Granular Engine");
            g.add(speed.set("speed", -0.5, -2.0, 2.0));
            g.add(rate.set("rate", 0.5, -2.0, 2.0));
            g.add(grainLength.set("grainLength", 0.05, 0.001, 0.25));
            g.add(overlaps.set("overlaps", 2, 1, 10));
            gui.add(g);
            
            ofParameterGroup g2;
            
            gui.loadFromFile("settings.xml");
        }
    
        void update() {
        
        }
    
        void draw() {
            ofBackground(0);
            
            gui.draw();
        }
    
        void mousePressed(ofMouseEventArgs args) {
            gui.mousePressed(args);
        }
    
        void audioOut(float *buffer, int buffer_size, int n_channels) {
            for (int i = 0; i < buffer_size; i++) {
                buffer[i] = (*ts).play(speed, rate, grainLength, overlaps);
            }
        }

    private:
        maxiSample sample1;
        maxiTimePitchStretch<hannWinFunctor, maxiSample> *ts;
        ofParameter<float> rate, speed, grainLength;
        ofParameter<int> overlaps;
        ofxPanel gui;
};


int main( ){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
