#include "ofMain.h"
#include "ofxMaxim.h"


class App : public ofBaseApp{
public:
    void setup() {
        width = 640;
        height = 480;
        ofSetWindowShape(width, height);
        
        ofSetFrameRate(60);
        ofSetBackgroundAuto(false);
        
        voice = 0;
        for (int i = 0; i < 6; i++) {
            ADSR[i].setAttack(0);
            ADSR[i].setDecay(200);
            ADSR[i].setSustain(0.2);
            ADSR[i].setRelease(2000);
        }
        
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
    }
    
    float play() {
        
        // we're adding up the samples each update and it makes sense to clear them each time first.
        mix = 0;
        
        // so this first bit is just a basic metronome so we can hear what we're doing.
        // this sets up a metronome that ticks 6 times a second
        currentCount = (int)timer.phasor(6);
        
        // if we have a new timer int this sample, play the sound
        if (lastCount != currentCount) {
            if (voice == 6) {
                voice = 0;
            }
            
            // trigger the envelope from the start
            ADSR[voice].trigger = 1;
            pitch[voice] = voice+1;
            voice++;
        }
        
        // and this is where we build the synth
        for (int i = 0; i < 6; i++) {
            
            //our ADSR env is passed a constant signal of 1 to generate the transient.
            ADSRout[i] = ADSR[i].adsr(1., ADSR[i].trigger);
            
            // this lfo is a sinewave at 0.1 hz
            LFO1out[i] = LFO1[i].sinebuf(0.1);
            
            // here's VCO1. it's a pulse wave at mouseX hz, with a pulse width of 0.6
            VCO1out[i] = VCO1[i].pulse(mouseX * pitch[i], 0.6);
            
            // here's VCO2. it's a pulse wave at mouseY hz with LFO modulation on the frequency width of 0.2
            VCO2out[i] = VCO2[i].pulse((mouseY * pitch[i]) + LFO1out[i], 0.2);
            
            // now we stick the VCO's into the VCF, using the ADSR as the filter cutoff
            VCFout[i] = VCF[i].lores((VCO1out[i] + VCO2out[i]) * 0.5, 250 + ((pitch[i] + LFO1out[i]) * 1000), 10);
            
            // finally we add the ADSR as an amplitude modulator
            mix += VCFout[i] * ADSRout[i] / 6;
        }
        
        // This just sends note-off messages.
        for (int i = 0; i < 6; i++) {
            ADSR[i].trigger = 0;
        }
        
        return mix;
    }
    
    void audioOut(float * output, int buffer_size, int n_channels) {
        for (int i = 0; i < buffer_size; i++) {
            output[i] = play();
        }
    }
    

private:
    int                     width,
                            height;
    
    int                     sample_rate,
                            buffer_size;
    
    float                   VCO1out[6],
                            VCO2out[6],
                            LFO1out[6],
                            LFO2out[6],
                            VCFout[6],
                            ADSRout[6],
                            mix,
                            pitch[6];
    maxiOsc                 VCO1[6], VCO2[6], LFO1[6], LFO2[6];
    maxiFilter              VCF[6];
    maxiEnv                 ADSR[6];
    maxiOsc                 timer;
    
    int                     currentCount, lastCount, voice;
};


int main() {
	ofSetupOpenGL(1024, 768, OF_WINDOW);
	ofRunApp(new App());
}
