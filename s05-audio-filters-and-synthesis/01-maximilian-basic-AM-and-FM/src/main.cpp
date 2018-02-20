#include "ofMain.h"
#include "ofxMaxim.h"


class App : public ofBaseApp{
public:
    void setup() {
        patch = 0;
        width = 640;
        height = 480;
        ofSetWindowShape(width, height);
        
        ofSetFrameRate(60);
        ofSetBackgroundAuto(false);
        
        sample_rate = 44100;
        buffer_size = 256;
        maxiSettings::setup(sample_rate, 1, buffer_size);
        ofSoundStreamSetup(1, 0, sample_rate, buffer_size, 3);
    }
    
    // i get called in a loop that runs until the program ends
    void update() {
        
    }
    
    // i also get called in a loop that runs until the program ends
    void draw() {
        ofBackground(0);
        switch(patch) {
            case '1':
                ofDrawBitmapString("1. Basic Oscillator", 20, 20);
                break;
            case '2':
                ofDrawBitmapString("2. Amplitude Modulation", 20, 20);
                break;
            case '3':
                ofDrawBitmapString("3. Amplitude Modulation w/ Phasor", 20, 20);
                break;
            case '4':
                ofDrawBitmapString("4. Frequency Modulation", 20, 20);
                break;
            default:
                break;
        }
    }
    
    void keyPressed(int k) {
        patch = k;
    }
    
    void ex_1_osc(float * output, int buffer_size) {
        for(int i = 0; i < buffer_size; i++) {
            output[i] = osc1.sinewave(mouseX);
        }
    }
    
    void ex_2_AM(float * output, int buffer_size) {
        for(int i = 0; i < buffer_size; i++) {
            output[i] = osc1.sinewave(mouseX) * osc2.sinewave(mouseY);
        }
    }
    
    void ex_3_AM(float * output, int buffer_size) {
        for(int i = 0; i < buffer_size; i++) {
            output[i] = osc1.sinewave(mouseX) * osc2.sinewave(osc3.phasor(0.01, 0, 440.0));
        }
    }
    
    void ex_4_FM(float * output, int buffer_size) {
        for(int i = 0; i < buffer_size; i++) {
            output[i] = osc1.sinewave(osc2.sinewave(mouseX) * mouseY + 440.0);
        }
    }
    
    
    void audioOut(float * output, int buffer_size, int n_channels) {
        switch(patch) {
            case '1':
                ex_1_osc(output, buffer_size);
                break;
            case '2':
                ex_2_AM(output, buffer_size);
                break;
            case '3':
                ex_3_AM(output, buffer_size);
                break;
            case '4':
                ex_4_FM(output, buffer_size);
                break;
            default:
                break;
        }
    }
    

private:
    int                     width,
                            height;
    
    int                     patch;
    
    vector<float>           audio_input;
    
    int                     sample_rate,
                            buffer_size;
    
    maxiOsc                 osc1, osc2, osc3;
};


int main() {
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new App());
}
