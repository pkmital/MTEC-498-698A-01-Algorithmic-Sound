#include "ofMain.h"
#include "ofxMaxim.h"
#include "maxiGrains.h"
#include "ofxGui.h"

class ofApp : public ofBaseApp{

    public:
        void setup(){
            width = 1024;
            height = 256;
            ofSetWindowShape(width, height * 2);
            
            setupAudio();
        }
    
        void setupAudio(){
            // openframeworks audio setup
            sample_rate = 44100;
            ofSoundStreamListDevices();
            ofSoundStreamSetup(0, 1, sample_rate, 512, 3);
            
            // maximilian setup
            maxiSettings::setup(sample_rate, 1, 3);
            
            // fft setup
            fft_size = 2048;
            window_size = fft_size / 2;
            hop_size = fft_size / 4;
            fft.setup(fft_size, window_size, hop_size);
        }
    
        void update() {
        
        }
    
        void draw(){
            ofBackground(40);
            ofPushMatrix();
            ofEnableAlphaBlending();
            
            ofSetColor(200, 200, 200, 220);
            ofDrawBitmapString("Magnitudes", 20, 20);
            ofSetColor(200, 100, 100, 220);
            drawMagnitudes(false);
            
            ofTranslate(0, height);
            ofSetColor(200, 200, 200, 220);
            ofDrawBitmapString("Magnitudes (decibel)", 20, 20);
            ofSetColor(100, 200, 100, 220);
            drawMagnitudes(true);
            
            ofDisableAlphaBlending();
            ofPopMatrix();
        }
    
        void drawMagnitudes(bool db) {
            float scale = 10;
            float ratio = width / (fft_size / 2.0f);
            float *mags;
            if (db)
                mags = fft.magsToDB();
            else
                mags = fft.magnitudes;
            for (int i = 0; i < fft_size / 2; i++) {
                ofDrawRectangle(
                    (float)i * ratio,
                    height,
                    ratio,
                    -mags[i] * scale);
            }
        }
    
        void audioIn(float *buffer, int buffer_size, int n_channels) {
            for(int i = 0; i < buffer_size * n_channels; i += n_channels)
            {
                fft.process(buffer[i]);
            }
        }

    private:
        int width;
        int height;
        maxiFFT fft;
        int sample_rate,
            fft_size,
            window_size,
            hop_size;
};


int main( ){
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new ofApp());
}
